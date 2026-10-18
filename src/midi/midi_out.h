#pragma once

#include <stdint.h>

void midi_send_note_on(uint8_t note, uint8_t velocity);
void midi_send_note_off(uint8_t note);
void midi_send_cc(uint8_t controller, uint8_t value);