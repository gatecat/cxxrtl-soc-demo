#include <arch/io.h>
#include <config.h>
#include <stdint.h>
#include <timer.h>

// TODO: enforce CONFIG_TIMER_CTR_WIDTH

struct timer_regs {
	uint32_t reload;
	uint32_t en;
	uint32_t ctr;
	uint32_t zero0; // reserved
	uint32_t zero1; // reserved
	uint32_t ev_status;
	uint32_t ev_pending;
	uint32_t ev_enable;
};

static inline void *timer_baseptr(void)
{
	uintptr_t timer_base = CONFIG_TIMER_START;
	return (void *)timer_base;
}

void timer_enable(uint32_t enable)
{
	struct timer_regs *regs = timer_baseptr();
	write32(&regs->en, enable);
}

void timer_reload(uint32_t value)
{
	struct timer_regs *regs = timer_baseptr();
	write32(&regs->reload, value);
}

void timer_set_count(uint32_t value)
{
	struct timer_regs *regs = timer_baseptr();
	write32(&regs->ctr, value);
}

uint32_t timer_get_count(void)
{
	struct timer_regs *regs = timer_baseptr();
	return read32(&regs->ctr);
}
