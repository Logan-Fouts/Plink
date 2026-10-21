#pragma once

#include <stdint.h>


void mux_hw_init(void);
void mux_hw_set_select_pin(int pin_ix, int level);
uint16_t mux_hw_read_adc(void);

