#ifndef __GENERATED_SOC_H_LAMBDASOC
#define __GENERATED_SOC_H_LAMBDASOC

#include <litex_config.h>
#include <software/include/generated/soc.h>

// FIXME: arch dependant !
#define CONFIG_CPU_NOP "nop"
static inline const char * config_cpu_nop_read(void) {
	return "nop";
}

#define CONFIG_L2_SIZE LX_CONFIG_SDRAM_CACHE_SIZE
static inline int config_l2_size_read(void) {
        return CONFIG_L2_SIZE;
}
#define MEMTEST_DATA_SIZE LX_CONFIG_MEMTEST_DATA_SIZE
static inline int memtest_data_size_read(void) {
	return MEMTEST_DATA_SIZE;
}
#define MEMTEST_ADDR_SIZE LX_CONFIG_MEMTEST_ADDR_SIZE
static inline int memtest_addr_size_read(void) {
        return MEMTEST_ADDR_SIZE;
}

#endif
