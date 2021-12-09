#ifndef __IO_H
#define __IO_H

#include <stdint.h>

static inline uint8_t read8(const void *addr)
{
	return *(volatile uint8_t *)addr;
}

static inline uint16_t read16(const void *addr)
{
	return *(volatile uint16_t *)addr;
}

static inline uint32_t read32(const void *addr)
{
	return *(volatile uint32_t *)addr;
}

static inline void write8(void *addr, uint8_t value)
{
	*(volatile uint8_t *)addr = value;
}

static inline void write16(void *addr, uint16_t value)
{
	*(volatile uint16_t *)addr = value;
}

static inline void write32(void *addr, uint32_t value)
{
	*(volatile uint32_t *)addr = value;
}

#endif
