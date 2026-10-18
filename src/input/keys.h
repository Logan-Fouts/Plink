#pragma once

#include <stdint.h>

// Set up all keys defined in config.h.
void keys_init(void);

// Scan every key and send MIDI on press/release.
void keys_update(uint32_t now_ms);