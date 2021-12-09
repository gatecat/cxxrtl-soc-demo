#ifndef __BOOT_COMMON_H
#define __BOOT_COMMON_H

#include <stdint.h>

void __attribute__((noreturn)) boot(uint32_t r1, uint32_t r2, uint32_t r3, uint32_t addr);

#endif
