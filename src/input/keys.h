#pragma once

#include <stdint.h>

// Set up the mux
void keys_init(void);

// Calibrate every key's resting value. Keep all keys released while this runs.
void apply_avg_reading(void);

// Read every key and send MIDI on press/release.
void keys_update(uint32_t now_us);

void   keys_shift_transpose(int8_t semitones);  // relative change
void   keys_set_transpose(int8_t semitones);    // absolute value
int8_t keys_get_transpose(void);
