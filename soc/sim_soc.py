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

