#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "config.h"


void utils_hw_sleep(uint32_t us);
void utils_hw_debug_write(char buf[DEBUG_BUFFER_SIZE]);
bool utils_hw_tud_conencted(void);
uint32_t time_us(void);
