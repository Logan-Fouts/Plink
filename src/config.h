#pragma once

// ---- Keys ----
// Each entry is { mux channel, MIDI note }.
#define KEY_COUNT           1
#define KEY_MAP             { {0, 60} }

// ---- Analog multiplexer (CD74HC4067) ----
#define MUX_SELECT_PINS     { 10, 11, 12, 13 }   // S0..S3
#define MUX_SELECT_PIN_COUNT 4
#define MUX_ADC_GPIO        26                   // SIG -> GP26 (ADC0)
#define MUX_SETTLE_US       10
#define MUX_SAMPLES         4                    // ADC reads averaged per key

// ---- Hall sensor / key travel ----
// Position is 0..1000 (per mille of travel).
#define HALL_THRESHOLD_LOW      200   // timing starts when travel passes this
#define HALL_THRESHOLD_HIGH     700   // note is sent when travel passes this
#define HALL_THRESHOLD_RELEASE  150   // key counts as released below this

// Expected ADC change from rest to fully pressed (12-bit counts, 0-4095).
// Positive if the reading RISES when pressed, negative if it FALLS.
// Use a conservative (small) value: the real span is learned automatically
// whenever a key goes further than this.
#define HALL_DEFAULT_SPAN       400
#define HALL_CALIBRATION_READS  32    // readings averaged for the resting value

// ---- Velocity ----
// Time between LOW and HIGH thresholds -> velocity. Tune with real data.
#define VELOCITY_FAST_US    2000      // this fast or faster = 127
#define VELOCITY_SLOW_US    60000     // this slow or slower = 1

// ---- MIDI ----
#define MIDI_CHANNEL        0         // 0 = channel 1

// ---- Debug ----
// Stream raw ADC + position of this key over serial every 20 ms
// (useful for tuning). Set to -1 to turn it off.
#define DEBUG_STREAM_KEY    0

// ---- LEDs (NeoPixel / WS2812, one per key) ----
#define LED_PIN             16
#define LED_COUNT           KEY_COUNT
#define LED_BRIGHTNESS      64            // 0-255, global (full white draws ~60 mA per LED)
#define LED_PRESSED_R       0
#define LED_PRESSED_G       120
#define LED_PRESSED_B       255
