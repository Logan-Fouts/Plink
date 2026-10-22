#pragma once

#include <stdint.h>

// Set up the mux
void keys_init(void);

// Calibrate every key's resting value. Keep all keys released while this runs.
void apply_avg_reading(void);

// Read every key and send MIDI on press/release.
void keys_update(uint32_t now_us);

