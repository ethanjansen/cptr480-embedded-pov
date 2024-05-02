#ifndef DROTENC_H
#define DROTENC_H

// Control the RGB LED using the Rotary Encoder: RED is clockwise (positive), BLUE is counter-clockwise (negative).
// Also outputs position via UART.

// Everything is static, here is why:
// 1. Only one rotary encoder.
// 2. Better handling of GPIO interrupts via callbacks (they require static members, use of <functional>, or perhaps cursed void pointers).
// 3. No need to create an instance of this class. (Is this good, IDK?)
// 4. Looking forward to how you grade this :)
class DROTENC {
    public:
        // Public API:

        // Initialize the rotary encoder
        // Initializes DGPIO, DTPM, and DUART -- configs must be set appropriately for each driver class.
        static void init();

        // returns the current position of the rotary encoder.
        // positive values are clockwise, negative values are counter-clockwise.
        inline static signed int getPosition() { return _position/POSITION_MULTIPLIER; }

        // sets the current position of the rotary encoder to 0.
        // This will be called via GPIO interrupt if should reset.
        static void resetPosition();

        // Controls position based on rotary encoder and internal state.
        // Called by GPIO interrupt.
        static void controlPosition();

        // Increment the current position of the rotary encoder.
        // This will be called via GPIO interrupt if should increment.
        static void incrementPosition();

        // Decrement the current position of the rotary encoder.
        // This will be called via GPIO interrupt if should decrement.
        static void decrementPosition();


    private:
        // no copy or assignment or constructor
        DROTENC();
        DROTENC(const DROTENC&);
        void operator=(const DROTENC&);

        // private API:

        enum {
            MAX_POSITION = 100, // ~1 (use 96 for perfect match) rotation for 100% duty cycle
            POSITION_MULTIPLIER = 653,
            MAX_POSITION_INTERNAL = POSITION_MULTIPLIER*MAX_POSITION, // used to match TPM frequency choice
        };

        enum ROTENCState {
            STATE_00,
            STATE_01,
            STATE_11,
            STATE_10,
            NUM_STATES,
        };

        // private position
        static signed int _position; // static because only one rotary encoder (stepped by +/- 653 to match TPM frequency choice)

        // private state
        static ROTENCState _state;

        // This will call DTPM::setCnV() based on _position (I want to use quadratic scaling).
        // Called by resetPosition(), incrementPosition(), and decrementPosition().
        static void _changeTPM();

        // This will call DUART::send() to send the current position to the UART.
        // Called by resetPosition(), incrementPosition(), and decrementPosition().
        static void _sendPosition();

        // This will set GPIOC values to extend/retract light bar (WIP)
        // Called by resetPosition(), incrementPosition(), and decrementPosition().
        static void _changeLightBar();
};

#endif // DROTENC_H