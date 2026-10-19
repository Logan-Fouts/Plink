# DevDocs
Here I will keep track of and work out the logic of the system including logic flow, objects(structs), functions, etc.


## Objects (structs)

### Button
- **button_t**: Represents a single button with its calibration and state. Contains (button_state_t)
    - **button_state_t**: State machine of button idle, travelling, and down.

- **button_event_t**: Holds the event type, velocity, and travel time. Contains (button_event_type_t)
    - **button_event_type_t**: 3 types of events for the button (none, pressed, released)

### Keys
- **keys**: Represents the array of keys on the keyboard. Contains (an array of keyswitch_t)
    - **keyswitch_t**: Represents a single key with its mux channel, note, and button. Contains (button_t)

## Logic Flow

1. Initialize USB and input/output devices
2. Initialize Keys which involves:
    1. Set up mux control with 4 pins and adc input from mux
    2. Set up neopixels
3. Calibrate each keys resting value and initalize each keys button with that value. **Note**: This must be done while no keys are pressed!
4. Read and clear incoming midi and cdc data.
5. Update the state of the keys array which invloves:
    1. Read raw value of each hall button by changing to its respective channel first and taking an average reading
    2. Updating button state with current reading and current time which invlovles:
        1. Compute button position and determine its state by its current state and current position and return corect button_event_t struct with event type, velocity, and travel time
    3. Sending midi note and setting led buffer to on or off based of current event type and related values
6. Update neopixels with the led buffer
