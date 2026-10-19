#pragma once

#include <stdint.h>

// --- MIDI Status Byte Definitions ---
// Control Change (CC) messages always start with 0xB (177 decimal)
#define MIDI_STATUS_CHANNEL_CONTROL 0xB0
// Note On/Off messages start with 0x9 and 0x8 respectively
#define MIDI_STATUS_NOTE_ON 0x90
#define MIDI_STATUS_NOTE_OFF 0x80

void midi_send_note_on(uint8_t note, uint8_t velocity);
void midi_send_note_off(uint8_t note);
void midi_send_cc(uint8_t controller, uint8_t value);