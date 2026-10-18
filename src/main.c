#include "pico/stdlib.h"
#include "tusb.h"

#include "input/keys.h"

int main(void) {
    tusb_init();
    keys_init();

    while (true) {
        tud_task();                          // USB housekeeping, must run often

        // Discard incoming data so the buffers never fill up
        uint8_t packet[4];
        while (tud_midi_available()) tud_midi_packet_read(packet);
        while (tud_cdc_available())  tud_cdc_read_char();

        keys_update(to_ms_since_boot(get_absolute_time()));
    }
}