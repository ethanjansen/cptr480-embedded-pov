#ifndef DROTENC_H
#define DROTENC_H

// Control the RGB LED using the Rotary Encoder: RED is clockwise (positive), BLUE is counter-clockwise (negative).
// Also outputs position via UART.
class DROTENC {
    public:
        // empty constructor
        DROTENC() {}

        // Public API:

        // Initialize the rotary encoder
        // Initializes DGPIO, DTPM, and DUART -- configs must be set appropriately for each driver class.
        void init();

        // returns the current position of the rotary encoder.
        // positive values are clockwise, negative values are counter-clockwise.
        inline signed int getPosition() { return _position/653; }

        // sets the current position of the rotary encoder to 0.
        // This will be called via GPIO interrupt if should reset.
        void resetPosition();

        // Increment the current position of the rotary encoder.
        // This will be called via GPIO interrupt if should increment.
        void incrementPosition();

        // Decrement the current position of the rotary encoder.
        // This will be called via GPIO interrupt if should decrement.
        void decrementPosition();


    private:
        // no copy or assignment
        DROTENC(const DROTENC&);
        void operator=(const DROTENC&);

        // private position
        static signed int _position; // static because only one rotary encoder (stepped by +/- 653 to match TPM frequency choice)

        // private API:

        enum {
            MAX_POSITION = 100, // ~1 (use 96 for perfect match) rotation for 100% duty cycle
            MAX_POSITION_INTERNAL = 653*MAX_POSITION, // used to match TPM frequency choice
        };

        // This will call DTPM::setCnV() based on _position (I want to use quadratic scaling).
        // Called by resetPosition(), incrementPosition(), and decrementPosition().
        void _changeTPM();

        // This will call DUART::send() to send the current position to the UART.
        // Called by resetPosition(), incrementPosition(), and decrementPosition().
        void _sendPosition();
};

// Single instance
extern DROTENC g_rotenc;

#endif // DROTENC_H