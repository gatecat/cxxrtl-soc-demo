#ifndef __IRQ_H
#define __IRQ_H

#include <arch/encoding.h>
#include <config.h>
#include <stdint.h>

static inline uint32_t irq_getie(void)
{
	return (read_csr(mstatus) & MSTATUS_MIE) != 0;
}

static inline void irq_setie(uint32_t ie)
{
	if (ie) {
		set_csr(mstatus, MSTATUS_MIE);
	} else {
		clear_csr(mstatus, MSTATUS_MIE);
	}
}

// TODO #include CPU-specific headers instead
#ifdef CONFIG_CPU_MINERVA
static inline uint32_t irq_getmask(void)
{
	return read_csr(0x330);
}

static inline void irq_setmask(uint32_t value)
{
	write_csr(0x330, value);
}

static inline uint32_t irq_pending(void)
{
	return read_csr(0x360);
}
#else
#error Unknown CPU
#endif

#endif
