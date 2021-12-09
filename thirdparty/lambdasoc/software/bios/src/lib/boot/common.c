#include <arch/irq.h>
#include <stdint.h>
#include <stdio.h>
#include <uart.h>

extern void boot_helper(uint32_t r1, uint32_t r2, uint32_t r3, uint32_t addr);

void __attribute__((noreturn)) boot(uint32_t r1, uint32_t r2, uint32_t r3, uint32_t addr)
{
	printf("Executing booted program.\n");
	uart_sync();
	irq_setmask(0);
	irq_setie(0);
        boot_helper(r1, r2, r3, addr);
	while(1);
}
