from amaranth import *
from amaranth.utils import log2_int

from thirdparty.amaranth_soc import wishbone
from thirdparty.amaranth_soc.memory import MemoryMap

from thirdparty.lambdasoc.periph import Peripheral

__all__ = ["SPIMemIO"]


class SPIMemIO(Peripheral, Elaboratable):
    """SRAM storage peripheral.
    Parameters
    ----------
    flash : dict
        QSPI Flash pins
    Attributes
    ----------
    bus : :class:`amaranth_soc.wishbone.Interface`
        Wishbone bus interface.
    """
    def __init__(self, *, flash, **kwargs):
        super().__init__()

        self.data_bus = wishbone.Interface(addr_width=22,
                                      data_width=32, granularity=8)
        self.flash = flash
        map = MemoryMap(addr_width=24, data_width=8)
        map.add_resource("flash", size=2**24)
        self.data_bus.memory_map = map

        bank            = self.csr_bank()
        self._cfgreg     = [bank.csr(8, "rw", name=f"cfg{i}") for i in range(4)]
        self._bridge    = self.bridge(data_width=32, granularity=8)
        self.ctrl_bus = self._bridge.bus

        self.size = 2**24

    def elaborate(self, platform):
        m = Module()
        spi_ready = Signal()
        # TODO : QSPI
        oe = Signal(4)
        cfgreg_do = Signal(32)
        m.submodules.spimemio = Instance(
            "spimemio",
            i_clk=ClockSignal(),
            i_resetn=~ResetSignal(),
            i_valid=self.data_bus.cyc&self.data_bus.stb,
            o_ready=spi_ready,
            i_addr=Cat(Const(0, 2), self.data_bus.adr), # Hack to force a 1MB offset
            o_rdata=self.data_bus.dat_r,
            o_flash_csb=self.flash["csn_o"],
            o_flash_clk=self.flash["clk_o"],
            o_flash_io0_oe=oe[0],
            o_flash_io1_oe=oe[1],
            o_flash_io2_oe=oe[2],
            o_flash_io3_oe=oe[3],
            o_flash_io0_do=self.flash["d0_o"],
            o_flash_io1_do=self.flash["d1_o"],
            o_flash_io2_do=self.flash["d2_o"],
            o_flash_io3_do=self.flash["d3_o"],
            i_flash_io0_di=self.flash["d0_i"],
            i_flash_io1_di=self.flash["d1_i"],
            i_flash_io2_di=self.flash["d2_i"],
            i_flash_io3_di=self.flash["d3_i"],
            i_cfgreg_we=Cat(self._cfgreg[i].w_stb for i in range(4)),
            i_cfgreg_di=Cat(self._cfgreg[i].w_data for i in range(4)),
            o_cfgreg_do=cfgreg_do,
        )
        m.submodules.bridge  = self._bridge
        for i in range(4): m.d.comb += self.flash[f"d{i}_oeb"].eq(~oe[i])
        m.d.comb += [
            self.flash["csn_oeb"].eq(0),
            self.flash["clk_oeb"].eq(0),
        ]
        m.d.comb += [self._cfgreg[i].r_data.eq(cfgreg_do[i*8:(i+1)*8])]

        # From https://github.com/im-tomu/foboot/blob/master/hw/rtl/picorvspi.py
        read_active = Signal()
        with m.If(self.data_bus.stb & self.data_bus.cyc & ~read_active):
            m.d.sync += read_active.eq(1)
            m.d.sync += self.data_bus.ack.eq(0)
        with m.Elif(read_active & spi_ready):
            m.d.sync += read_active.eq(0)
            m.d.sync += self.data_bus.ack.eq(1)
        with m.Else():
            m.d.sync += self.data_bus.ack.eq(0)

        return m
