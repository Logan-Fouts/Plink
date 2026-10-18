#include "pico/stdlib.h"
#include "tusb.h"

#include "input/keys.h"

int main(void) {
    tusb_init();
    keys_init();                             // calibrates: keep keys released

    while (true) {
        tud_task();                          // USB housekeeping, must run often

        // Discard incoming data so the buffers never fill up
        uint8_t packet[4];
        while (tud_midi_available()) tud_midi_packet_read(packet);
        while (tud_cdc_available())  tud_cdc_read_char();

        keys_update(time_us_32());           // microseconds, for velocity timing
    }
}