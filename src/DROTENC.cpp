#include "DROTENC.h"

// other classes for initalization
#include "DGPIO.h"
#include "DTPM.h"
#include "DUART.h"

// Single instances
DGPIO g_gpio;
DTPM g_tpm;
DUART g_uart;

// static variables
signed int DROTENC::_position = 0;
DROTENC::ROTENCState DROTENC::_state = STATE_00;

void DROTENC::init() {
    // initialize GPIO, TPM, and UART
    g_tpm.init();
    g_uart.init();
    g_gpio.Init(); // do this last for fear of interrupts during initialization
}

// sets the current position of the rotary encoder to 0.
// This will be called via GPIO interrupt if should reset.
void DROTENC::resetPosition() { 
    _position = 0;
    _changeTPM();
    _sendPosition();
}

// Controls position based on rotary encoder and internal state.
// Called by GPIO interrupt.
// Currently we are polling the ENCODER values via gpio Status(), but perhaps it would be better to check which interrupt was triggered...
void DROTENC::controlPosition() {
    switch (_state) {
        case STATE_00:
            if (g_gpio.Status(DGPIO::ENCODER2)) {
                incrementPosition();
                _state = STATE_01;
            } else { // don't bother checking other values, only one other change should be possible
                decrementPosition();
                _state = STATE_10;
            }
            return;
        case STATE_01:
            if (g_gpio.Status(DGPIO::ENCODER1)) {
                incrementPosition();
                _state = STATE_11;
            } else {
                decrementPosition();
                _state = STATE_00;
            }
            return;
        case STATE_11:
            if (!g_gpio.Status(DGPIO::ENCODER2)) {
                incrementPosition();
                _state = STATE_10;
            } else {
                decrementPosition();
                _state = STATE_01;
            }
            return;
        case STATE_10:
            if (!g_gpio.Status(DGPIO::ENCODER1)) {
                incrementPosition();
                _state = STATE_00;
            } else {
                decrementPosition();
                _state = STATE_11;
            }
            return;
        default:
            _state = STATE_00;
            return; // Oh how have we gotten here?
    }
}

// Increment the current position of the rotary encoder.
// This will be called via GPIO interrupt if should increment.
void DROTENC::incrementPosition() { 
    _position += 653; 
    _changeTPM();
    _sendPosition();
}

// Decrement the current position of the rotary encoder.
// This will be called via GPIO interrupt if should decrement.
void DROTENC::decrementPosition() { 
    _position -= 653; 
    _changeTPM();
    _sendPosition();
}

void DROTENC::_changeTPM() {
    // TODO: implement.
    // get absolute value for use with setCnV
    unsigned abs = _position > 0 ? _position : -_position;

    // if limit reached, do nothing
    if (abs > MAX_POSITION_INTERNAL) {
        return;
    } else if (_position < 0) {
        g_tpm.setCnV(DTPM::TPM_0, DTPM::TPM_CH1, ((abs * abs) >> 16)); // blue
    } else if (_position > 0) {
        g_tpm.setCnV(DTPM::TPM_2, DTPM::TPM_CH0, ((abs * abs) >> 16)); // red
    } else {
        // off
        g_tpm.setCnV(DTPM::TPM_2, DTPM::TPM_CH0, 0); // red
        g_tpm.setCnV(DTPM::TPM_0, DTPM::TPM_CH1, 0); // blue
    }
}

inline void DROTENC::_sendPosition() {
    g_uart.sendInt(_position/POSITION_MULTIPLIER, 10); // use base 10
}