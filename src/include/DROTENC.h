#ifndef DROTENC_H
#define DROTENC_H

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
        inline signed int getPosition() { return _position; }

        // sets the current position of the rotary encoder to 0.
        // This will be called via GPIO interrupt if should reset.
        inline void resetPosition() { 
            _position = 0; 
            _changeTPM();
            _sendPosition();
        }

        // Increment the current position of the rotary encoder.
        // This will be called via GPIO interrupt if should increment.
        inline void incrementPosition() { 
            _position++; 
            _changeTPM();
            _sendPosition();
        }

        // Decrement the current position of the rotary encoder.
        // This will be called via GPIO interrupt if should decrement.
        inline void decrementPosition() { 
            _position--; 
            _changeTPM();
            _sendPosition();
        }


    private:
        // no copy or assignment
        DROTENC(const DROTENC&);
        void operator=(const DROTENC&);

        // private position
        signed int _position = 0;

        // private API:

        // This will call DTPM::setDutyCycle() based on _position.
        // Called by resetPosition(), incrementPosition(), and decrementPosition().
        void _changeTPM();

        // This will call DUART::send() to send the current position to the UART.
        // Called by resetPosition(), incrementPosition(), and decrementPosition().
        void _sendPosition();
};

// Single instance
extern DROTENC g_rotenc;

#endif // DROTENC_H