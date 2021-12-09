#ifndef __GENERATED_CSR_H_LAMBDASOC
#define __GENERATED_CSR_H_LAMBDASOC

#include <litex_config.h>
#include <generated/mem.h>

#include <software/include/generated/csr.h>

// LiteX's memspeed() requires timer accessors.

#define TIMER_RELOAD_ADDR (LX_CONFIG_TIMER_START + 0x0)
#define TIMER_EN_ADDR     (LX_CONFIG_TIMER_START + 0x4)
#define TIMER_CTR_ADDR    (LX_CONFIG_TIMER_START + 0x8)

static inline uint32_t timer0_load_read(void) {
	return csr_read_simple(TIMER_RELOAD_ADDR);
}
static inline void timer0_load_write(uint32_t v) {
	csr_write_simple(v, TIMER_RELOAD_ADDR);
}
static inline uint32_t timer0_reload_read(void) {
	return csr_read_simple(TIMER_RELOAD_ADDR);
}
static inline void timer0_reload_write(uint32_t v) {
	csr_write_simple(v, TIMER_RELOAD_ADDR);
}
static inline uint32_t timer0_en_read(void) {
	return csr_read_simple(TIMER_EN_ADDR);
}
static inline void timer0_en_write(uint32_t v) {
	csr_write_simple(v, TIMER_EN_ADDR);
}

static uint32_t __timer0_value_saved = 0;

static inline uint32_t timer0_update_value_read(void) {
	return 0;
}
static inline void timer0_update_value_write(uint32_t v) {
        if (!!v) {
		__timer0_value_saved = csr_read_simple(TIMER_CTR_ADDR);
        }
}
static inline uint32_t timer0_value_read(void) {
	return __timer0_value_saved;
}

#endif
