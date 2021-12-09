top := $(CURDIR)
src := $(top)/src
build ?= build
obj := $(build)/bios

kconfig-src := $(top)/util/kconfig
kconfig-obj := $(build)/util/kconfig

HOSTCC  := gcc
HOSTCXX := g++

KCONFIG_CONFIG ?= $(top)/.config
KCONFIG_AUTOHEADER := $(build)/include/config.h
KCONFIG_AUTOCONFIG := $(build)/auto.conf
KCONFIG_DEPENDENCIES := $(build)/auto.conf.cmd
KCONFIG_SPLITCONFIG := $(build)/config
KCONFIG_TRISTATE := $(build)/tristate.conf
KCONFIG_NEGATIVES := 1
KCONFIG_STRICT := 1

BIOS_EXPORTS := BIOS_EXPORTS
BIOS_EXPORTS += KCONFIG_CONFIG KCONFIG_AUTOHEADER KCONFIG_AUTOCONFIG
BIOS_EXPORTS += KCONFIG_DEPENDENCIES KCONFIG_SPLITCONFIG KCONFIG_TRISTATE
BIOS_EXPORTS += KCONFIG_NEGATIVES KCONFIG_STRICT

export $(BIOS_EXPORTS)

all: $(KCONFIG_AUTOHEADER) $(obj)/bios.bin

include $(kconfig-src)/build.mk

$(KCONFIG_AUTOHEADER): $(KCONFIG_CONFIG)
	+$(MAKE) oldconfig

$(KCONFIG_AUTOCONFIG): $(KCONFIG_AUTOHEADER)

-include $(KCONFIG_CONFIG)

# Include a Makefile.
# Append obj-y to objs, and subdir-y to subdirs.
# $1 : Makefile
include-makefile = \
	$(eval obj-y :=) \
	$(eval -include $1) \
	$(eval objs += \
		$$(subst $(top)/,, \
		$$(abspath $$(subst $(dir $1)/,/,$$(addprefix $(dir $1),$$(obj-y)))))) \
	$(eval subdirs+=$$(subst $(CURDIR)/,,$$(wildcard $$(abspath $$(addprefix $(dir $1),$$(subdir-y))))))

visit-subdirs = \
  	$(eval cursubdirs := $(subdirs)) \
	$(eval subdirs :=) \
	$(foreach dir,$(cursubdirs), \
		$(eval $(call include-makefile,$(dir)/Makefile))) \
	$(if $(subdirs),$(eval $(call visit-subdirs)))

# Collect all object files eligible for building
objs :=
subdirs := src
$(eval $(call visit-subdirs))

# Eliminate duplicate mentions of object files
objs := $(sort $(objs))
objs := $(patsubst src/%,$(obj)/%,$(objs))

dirs := $(abspath $(dir $(objs)))

include toolchain.mk

dirs := $(sort $(dirs))

$(shell mkdir -p $(KCONFIG_SPLITCONFIG) $(kconfig-obj)/lxdialog $(build)/include $(dirs))

clean:
	$(RM) -r $(obj)

distclean:
	$(RM) $(KCONFIG_CONFIG) $(KCONFIG_CONFIG).old
	$(RM) -r $(build)

.PHONY: all clean distclean
