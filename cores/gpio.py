from amaranth import *

from thirdparty.lambdasoc.periph import Peripheral

__all__ = ["GPIOPeripheral"]


class GPIOPeripheral(Peripheral, Elaboratable):
    def __init__(self, width, pins, **kwargs):
        super().__init__()

        self.width = width
        self.pins = pins

        bank            = self.csr_bank()
        self._dout      = bank.csr(width, "rw")
        self._oe        = bank.csr(width, "rw")
        self._din       = bank.csr(width, "r")

        self._bridge    = self.bridge(data_width=32, granularity=8, alignment=2)
        self.bus        = self._bridge.bus

    def elaborate(self, platform):
        m = Module()
        m.submodules.bridge  = self._bridge

        dout_buf = Signal(self.width)
        oe_buf = Signal(self.width)

        with m.If(self._dout.w_stb): m.d.sync += dout_buf.eq(self._dout.w_data)
        with m.If(self._oe.w_stb): m.d.sync += oe_buf.eq(self._oe.w_data)

        m.d.comb += [
            self._dout.r_data.eq(dout_buf),
            self._oe.r_data.eq(oe_buf),
        ]

        for i in range(self.width):
            m.d.comb += [
                self.pins[f"{i}_o"].eq(dout_buf[i]),
                self.pins[f"{i}_oeb"].eq(~oe_buf[i]),
            ]
            m.d.sync += self._din.r_data[i].eq(self.pins[f"{i}_i"])

        return m
