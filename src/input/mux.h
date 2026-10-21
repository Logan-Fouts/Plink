#pragma once

#include <stdint.h>

// Set up the ADC pin and the 4 mux select pins.
void mux_init(void);

// Select a mux channel (0-15) and return the averaged 12-bit ADC reading.
uint16_t mux_read(int channel);
