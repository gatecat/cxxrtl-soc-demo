#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>

void timer_enable(uint32_t enable);
void timer_reload(uint32_t value);
void timer_set_count(uint32_t value);
uint32_t timer_get_count(void);

#endif
