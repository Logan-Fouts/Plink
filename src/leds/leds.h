#pragma once

#include <stdint.h>

// One NeoPixel (WS2812) per key. LED index = key index.
// leds_set_* only change a buffer; nothing is sent until leds_update().

void leds_init(void);

// Set one LED's colour (0-255 per channel; global brightness is applied).
void leds_set_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

void leds_off(uint8_t index);
void leds_clear(void);

// Send the buffer to the LEDs, but only if something changed and the
// previous frame has finished. Safe to call every loop pass.
void leds_update(void);