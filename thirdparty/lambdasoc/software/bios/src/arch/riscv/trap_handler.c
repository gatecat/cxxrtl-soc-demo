#include <arch/irq.h>
#include <config.h>
#include <stdio.h>
#include <stdint.h>
#include <uart.h>

struct trap_frame {
	struct _gpr {
		uint32_t ra;
		uint32_t t0;
		uint32_t t1;
		uint32_t t2;
		uint32_t a0;
		uint32_t a1;
		uint32_t a2;
		uint32_t a3;
		uint32_t a4;
		uint32_t a5;
		uint32_t a6;
		uint32_t a7;
		uint32_t t3;
		uint32_t t4;
		uint32_t t5;
		uint32_t t6;
	} gpr;
	uint32_t mepc;
	uint32_t mcause;
};

void trap_handler(struct trap_frame *tf);
void trap_handler(struct trap_frame *tf)
{
	if (tf->mcause & 0x80000000) {
		uint32_t irqs = irq_pending() & irq_getmask();

		if (irqs & (1 << CONFIG_UART_IRQNO)) {
			uart_isr();
		}
	} else {
		printf("Panic! at mepc=%08x (mcause=%08x)\n",
		       tf->mepc, tf->mcause);
	}
}
