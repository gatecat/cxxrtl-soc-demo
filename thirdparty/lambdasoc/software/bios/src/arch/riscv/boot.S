#define MIE_MEIE 0x800

	.section .text, "ax", @progbits
	.global boot_helper
boot_helper:
	// disable external interrupts
	li t0, MIE_MEIE
	csrc mie, t0

	// flush instruction cache
	fence.i

	// jump to payload
	jr a3
