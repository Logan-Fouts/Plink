#pragma once

// ---- Keys ----
// Each entry is { mux channel, MIDI note }.
// Start with ONE wired sensor while testing: unconnected mux inputs float
// and give random readings that can trigger phantom notes.
#define KEY_COUNT           1
#define KEY_MAP             { {0, 60} }

// Example for 15 keys (C4 up to D5, mux channels 0-14):
// #define KEY_COUNT        15
// #define KEY_MAP          { {0,60},{1,61},{2,62},{3,63},{4,64},{5,65},{6,66},{7,67}, \
//                            {8,68},{9,69},{10,70},{11,71},{12,72},{13,73},{14,74} }

// ---- Analog multiplexer (CD74HC4067) ----
#define MUX_SELECT_PINS     { 10, 11, 12, 13 }   // S0..S3
#define MUX_ADC_GPIO        26                   // SIG -> GP26 (ADC0)
#define MUX_SETTLE_US       10                   // wait after switching channel
#define MUX_SAMPLES         4                    // ADC reads averaged per key

// ---- Hall sensor / key travel ----
// Position is 0..1000 (per mille of travel).
#define HALL_THRESHOLD_LOW      200   // timing starts when travel passes this
#define HALL_THRESHOLD_HIGH     700   // note is sent when travel passes this
#define HALL_THRESHOLD_RELEASE  150   // key counts as released below this
                                      // (keep it below LOW so noise can't retrigger)

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