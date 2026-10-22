#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "config.h"


void utils_hw_sleep(uint32_t us);
void utils_hw_debug_write(char buf[DEBUG_BUFFER_SIZE]);
bool utils_hw_tud_connected(void);
uint32_t time_us(void);
void utils_hw_midi_stream_write(uint8_t cable_num, uint8_t msg[3], uint32_t bufsize);
