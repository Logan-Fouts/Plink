#include "midi/midi_out.h"

#include "tusb.h"
#include "config.h"

static void send3(uint8_t status, uint8_t d1, uint8_t d2) {
    if (!tud_mounted()) return;               // host not connected yet
    uint8_t msg[3] = { (uint8_t)(status | MIDI_CHANNEL), d1, d2 };
    tud_midi_stream_write(0, msg, 3);
}

void midi_send_note_on(uint8_t note, uint8_t velocity) {
    send3(MIDI_STATUS_NOTE_ON, note & 0x7F, velocity & 0x7F);
}

void midi_send_note_off(uint8_t note) {
    send3(MIDI_STATUS_NOTE_OFF, note & 0x7F, 0);
}

void midi_send_cc(uint8_t controller, uint8_t value) {
    send3(MIDI_STATUS_CHANNEL_CONTROL, controller & 0x7F, value & 0x7F);
}