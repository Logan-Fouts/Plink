#pragma once

// ---- Keys ----
#define KEY_COUNT           4
#define KEY_PINS            { 0, 1, 2, 3 }        // GPIO per key (switch to GND)
#define KEY_NOTES           { 60, 62, 64, 65 }    // C4, D4, E4, F4

// ---- MIDI ----
#define MIDI_CHANNEL        0                     // 0 = channel 1
#define DEFAULT_VELOCITY    100

// ---- Timing ----
#define BUTTON_DEBOUNCE_MS  10