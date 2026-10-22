# Plink
 
Base: simple, cheap MIDI keyboard (Pico, USB MIDI, keyswitches)
 
## Ideas
 
- Programmed in C (since need fast reads for key press velocity)
- Hall sensor Key Switches
- Potentiometers
- LEDs
- TX and RX MIDI
- Non-deterministic parametrized patterns (tbd)
 
## Parts List
*Inital parts list for v1.0*
- Pi Pico
- Analog Multiplexer (CD74HC4067, https://www.digikey.com/en/products/detail/texas-instruments/CD74HC4067SM96/390310)
- Halleffect Key Switches (https://www.maxgaming.se/sv/switchar/moyu-studio-yurui-he-linear-switch)
- Halleffect sensors (https://www.electrokit.com/en/ss49eua-to-92-hall-effektsensor-2mv/g)
- Neopixels (https://www.digikey.com/en/products/detail/adafruit-industries-llc/1655/5154679)

## Testing
- `cd ~/Code/Plink/`
- `cmake -S test -B build/test`
- `cmake --build build/test`
- `ctest --test-dir build/test --output-on-failure`

Intended to be expanded on as the project grows
