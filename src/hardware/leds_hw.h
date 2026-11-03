#pragma once

#include <stdint.h>

void leds_hw_init(void);
void leds_hw_pio_put_blocking(uint32_t pixel);
