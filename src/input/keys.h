#pragma once

#include <stdint.h>

// Set up the mux and calibrate every key's resting value.
// Keep all keys released while this runs.
void keys_init(void);

// Read every key and send MIDI on press/release. Time is in microseconds.
void keys_update(uint32_t now_us);