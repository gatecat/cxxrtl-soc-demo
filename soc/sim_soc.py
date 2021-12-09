import argparse
import importlib

from nmigen import *
from thirdparty.nmigen_soc import wishbone

from thirdparty.lambdasoc.cpu.minerva import MinervaCPU
from thirdparty.lambdasoc.periph.intc import GenericInterruptController
from thirdparty.lambdasoc.periph.serial import AsyncSerialPeripheral
from thirdparty.lambdasoc.periph.sram import SRAMPeripheral
from thirdparty.lambdasoc.periph.timer import TimerPeripheral
from thirdparty.lambdasoc.soc.cpu import CPUSoC

from cores.gpio import GPIOPeripheral
from cores.spimemio_wrapper import SPIMemIO
from cores.hyperram import HyperRAM

class HyperRamSoC(CPUSoC, Elaboratable):
    def __init__(self, *, reset_addr, clk_freq,
                 rom_addr, flash_ctrl_addr, flash_pins,
                 hram_addr, hyperram_pins,
                 sram_addr, sram_size,
                 uart_addr, uart_divisor, uart_pins,
                 timer_addr, timer_width,
                 gpio_addr, gpio_count, gpio_pins):
        self._arbiter = wishbone.Arbiter(addr_width=30, data_width=32, granularity=8)
        self._decoder = wishbone.Decoder(addr_width=30, data_width=32, granularity=8)

        self.cpu = MinervaCPU(reset_address=reset_addr, with_muldiv=True)
        self._arbiter.add(self.cpu.ibus)
        self._arbiter.add(self.cpu.dbus)

        self.rom = SPIMemIO(flash=flash_pins)
        self._decoder.add(self.rom.data_bus, addr=rom_addr)
        self._decoder.add(self.rom.ctrl_bus, addr=flash_ctrl_addr)

        self.ram = SRAMPeripheral(size=sram_size)
        self._decoder.add(self.ram.bus, addr=sram_addr)

        self.hyperram = HyperRAM(io=hyperram_pins)
        self._decoder.add(self.hyperram.bus, addr=hram_addr)

        self.uart = AsyncSerialPeripheral(divisor=uart_divisor, pins=uart_pins, rx_depth=4, tx_depth=4)
        self._decoder.add(self.uart.bus, addr=uart_addr)

        self.timer = TimerPeripheral(width=timer_width)
        self._decoder.add(self.timer.bus, addr=timer_addr)

        self.intc = GenericInterruptController(width=len(self.cpu.ip))
        self.intc.add_irq(self.timer.irq, 0)
        self.intc.add_irq(self.uart .irq, 1)

        self.gpio = GPIOPeripheral(gpio_count, gpio_pins)
        self._decoder.add(self.gpio.bus, addr=gpio_addr)

        self.memory_map = self._decoder.bus.memory_map

        self.clk_freq = clk_freq

    def elaborate(self, platform):
        m = Module()

        m.submodules.arbiter  = self._arbiter
        m.submodules.cpu      = self.cpu

        m.submodules.decoder  = self._decoder
        m.submodules.rom      = self.rom
        m.submodules.ram      = self.ram
        m.submodules.hyperram = self.hyperram
        m.submodules.uart     = self.uart
        m.submodules.timer    = self.timer
        m.submodules.gpio     = self.gpio
        m.submodules.intc     = self.intc

        m.d.comb += [
            self._arbiter.bus.connect(self._decoder.bus),
            self.cpu.ip.eq(self.intc.ip),
        ]

        return m

# Create a pretend UART resource with arbitrary signals
class UARTPins():
    class Input():
        def __init__(self, sig):
            self.i = sig
    class Output():
        def __init__(self, sig):
            self.o = sig
    def __init__(self, rx, tx):
        self.rx = UARTPins.Input(rx)
        self.tx = UARTPins.Output(tx)

class SimPeripheral(Elaboratable):

    verilog_boxes = dict()

    def reset_boxes():
        verilog_boxes = dict()

    def __init__(self, name, pins):
        self.name = name
        self.io = {}
        self.pins = [(p.replace(">", ""), w) for p, w in pins]
        for pin, w in self.pins:
            for i in range(w):
                bit_name = f"{pin}{i}" if w > 1 else pin
                self.io[f"{bit_name}_i"] = Signal()
                self.io[f"{bit_name}_o"] = Signal()
                self.io[f"{bit_name}_oeb"] = Signal()
        if name not in SimPeripheral.verilog_boxes:
            SimPeripheral.verilog_boxes[self.name] = f"(* blackbox, cxxrtl_blackbox, keep *) module {name} (\n"
            verilog_pins = []
            for pin, w in pins:
                bb_pin = "periph" if len(pin) == 0 else pin.replace(">", "")
                verilog_pins.append(f"    output [{w-1}:0] {bb_pin}_i")
                edge = '(* cxxrtl_edge="a" *)' if '>' in pin else ''
                verilog_pins.append(f"{edge}    input  [{w-1}:0] {bb_pin}_o")
                verilog_pins.append(f"    input  [{w-1}:0] {bb_pin}_oeb")
            SimPeripheral.verilog_boxes[self.name] += ",\n".join(verilog_pins)
            SimPeripheral.verilog_boxes[self.name] += "\n);\n"
            SimPeripheral.verilog_boxes[self.name] += "endmodule\n"

    def elaborate(self, platform):
        m = Module()
        conn = dict(a_keep=True)
        for pin, w in self.pins:
            sig_i = Signal(w)
            sig_o = Signal(w)
            sig_oeb = Signal(w)
            bb_pin = "periph" if len(pin) == 0 else pin
            conn[f"o_{bb_pin}_i"] = sig_i
            conn[f"i_{bb_pin}_o"] = sig_o
            conn[f"i_{bb_pin}_oeb"] = sig_oeb
            for i in range(w):
                bit_name = f"{pin}{i}" if w > 1 else pin
                m.d.comb += self.io[f"{bit_name}_i"].eq(sig_i[i])
                m.d.comb += sig_o[i].eq(self.io[f"{bit_name}_o"])
                m.d.comb += sig_oeb[i].eq(self.io[f"{bit_name}_oeb"])
        m.submodules.bb = Instance(self.name, **conn)
        return m

    def write_boxes(f):
        for _, box in sorted(SimPeripheral.verilog_boxes.items(), key=lambda x: x[0]):
            print(box, file=f)

# SoC top
class SimSoc(Elaboratable):
    def __init__(self, build_dir="build", with_bios=False):
        self.clk = Signal()
        self.rst = Signal()
        self.uart_tx = Signal()
        self.build_dir = build_dir
        self.with_bios = with_bios

    def elaborate(self, platform):
        m = Module()
        m.domains.sync = ClockDomain(async_reset=False)
        m.d.comb += ClockSignal().eq(self.clk)
        m.d.comb += ResetSignal().eq(self.rst)

        SimPeripheral.reset_boxes()

        # SPI flash
        spiflash = SimPeripheral("spiflash_model", [(">clk", 1), (">csn", 1), ("d", 4)])
        m.submodules.spiflash = spiflash

        # HyperRAM
        hyperram = SimPeripheral("hyperram_model", [(">clk", 1), (">rwds", 1), (">csn", 1), ("d", 8)])
        m.submodules.hyperram = hyperram

        # GPIO
        gpio = SimPeripheral("gpio_model", [("", 8), ])
        m.submodules.gpio = gpio

        # UART
        uart = SimPeripheral("uart_model", [("tx", 1), ("rx", 1)])
        m.submodules.uart = uart
        uart_pins = UARTPins(rx=uart.io["rx_i"], tx=uart.io["tx_o"])
        m.d.comb += self.uart_tx.eq(uart.io["tx_o"])

        # The SoC itself
        m.submodules.soc = HyperRamSoC(
            reset_addr=0x00100000, clk_freq=int(27e6),
            rom_addr=0x00000000, flash_ctrl_addr=0x10007000, flash_pins=spiflash.io,
            hram_addr=0x20000000, hyperram_pins=hyperram.io,
            sram_addr=0x10004000, sram_size=0x200,
            uart_addr=0x10005000, uart_divisor=int(27e6 // 9600), uart_pins=uart_pins,
            timer_addr=0x10006000, timer_width=32,
            gpio_addr=0x10008000, gpio_count=8, gpio_pins=gpio.io,
        )
        if self.with_bios:
            m.submodules.soc.build(build_dir=f"{self.build_dir}/soc", do_init=True)
        with open(f"{self.build_dir}/sim_blackboxes.v", "w") as f:
            SimPeripheral.write_boxes(f)
        return m

if __name__ == "__main__":
    sim_top = SimSoc()
    from nmigen.cli import main
    main(sim_top, name="sim_top", ports=[sim_top.clk, sim_top.rst, sim_top.uart_tx])
