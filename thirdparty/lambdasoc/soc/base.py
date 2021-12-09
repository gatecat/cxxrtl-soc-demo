import os
import re
import textwrap
import jinja2

from nmigen import tracer
from thirdparty.nmigen_soc.memory import MemoryMap
from nmigen.build.run import *

from .. import software
from ..periph import Peripheral


__all__ = ["socproperty", "SoC", "ConfigBuilder"]


def socproperty(cls, *, weak=False, src_loc_at=0):
    name   = tracer.get_var_name(depth=2 + src_loc_at)
    __name = "__{}".format(name)

    def getter(self):
        assert isinstance(self, SoC)
        attr = getattr(self, __name, None)
        if attr is None and not weak:
            raise NotImplementedError("SoC {!r} does not have a {}"
                                      .format(self, name))
        return attr

    def setter(self, value):
        assert isinstance(self, SoC)
        if not isinstance(value, cls):
            raise TypeError("{} must be an instance of {}, not {!r}"
                            .format(name, cls.__name__, value))
        setattr(self, __name, value)

    return property(getter, setter)


class SoC:
    memory_map = socproperty(MemoryMap)

    def build(self, build_dir="build/soc", do_build=True, name=None):
        plan = ConfigBuilder().prepare(self, build_dir, name)
        if not do_build:
            return plan

        products = plan.execute_local(build_dir)
        return products


class ConfigBuilder:
    file_templates = {
        "build_{{name}}.sh": r"""
            # {{autogenerated}}
            set -e
            {{emit_commands()}}
        """,
        "{{name}}_resources.csv": r"""
            # {{autogenerated}}
            # <resource name>, <start address>, <end address>, <access width>
            {% for resource, (start, end, width) in soc.memory_map.all_resources() -%}
                {{resource.name}}, {{hex(start)}}, {{hex(end)}}, {{width}}
            {% endfor %}
        """,
    }
    command_templates = []

    def prepare(self, soc, build_dir, name, **render_params):
        name = name or type(soc).__name__.lower()

        autogenerated = "Automatically generated by LambdaSoC {}. Do not edit.".format("__version__")

        def periph_addr(periph):
            assert isinstance(periph, Peripheral)
            periph_map = periph.bus.memory_map
            for window, (start, end, ratio) in soc.memory_map.windows():
                if periph_map is window:
                    return start
            raise KeyError(periph)

        def periph_csrs(periph):
            assert isinstance(periph, Peripheral)
            periph_map = periph.bus.memory_map
            for resource, (start, end, width) in periph_map.all_resources():
                if isinstance(resource, csr.Element):
                    yield resource, (start, end, width)

        def emit_commands():
            commands = []
            for index, command_tpl in enumerate(self.command_templates):
                command = render(command_tpl, origin="<command#{}>".format(index + 1))
                command = re.sub(r"\s+", " ", command)
                commands.append(command)
            return "\n".join(commands)

        def render(source, origin):
            try:
                source = textwrap.dedent(source).strip()
                compiled = jinja2.Template(source, trim_blocks=True, lstrip_blocks=True)
            except jinja2.TemplateSyntaxError as e:
                e.args = ("{} (at {}:{})".format(e.message, origin, e.lineno),)
                raise
            return compiled.render({
                "autogenerated": autogenerated,
                "build_dir": os.path.abspath(build_dir),
                "emit_commands": emit_commands,
                "hex": hex,
                "name": name,
                "periph_addr": periph_addr,
                "periph_csrs": periph_csrs,
                "soc": soc,
                "software_dir": os.path.dirname(software.__file__),
                **render_params,
            })

        plan = BuildPlan(script="build_{}".format(name))
        for filename_tpl, content_tpl in self.file_templates.items():
            plan.add_file(render(filename_tpl, origin=filename_tpl),
                          render(content_tpl, origin=content_tpl))
        return plan
