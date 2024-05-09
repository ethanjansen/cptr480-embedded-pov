// LED Class used for Lab 03

// self
#include "LED.h"
// others
#include "DGPIO.h"
#include "DTPM.h"
#include "DPIT.h"


// Private variable initialization
unsigned LED::_useExpStateTransition = 0;
LED::RGBState LED::_state = RED;
unsigned LED::_dutyCycle[] = {MAX_PERCENT_SQR, 0, 0}; // red starts on

// Empty constructor
LED::LED() {}

// Initialize GPIO, TPM, and PIT.
// Requires GPIO and TPM classes are configured correctly.
// Set speed to fade and to use exponential or square transitions.
void LED::init(unsigned speed, LEDPercents maxPercent) {
    // Initialize hardware via their classes
    if (!DGPIO::isInit()) { // set up TPM on LED_RED, LED_GREEN, LED_BLUE
        DGPIO::init();
    }
    if (!DTPM::isInit()) { // Set initial TPM values (match _dutyCycle init)
        DTPM::init();
    }
    if (!DPIT::isInit()) {
        DPIT::init();
    }

    // Set PIT interrupt frequency (how fast will fading occur)
    DPIT::setInterruptsPerSec(DPIT::PIT0, speed);

    // initialize duty Cycles
    if (maxPercent == MAX_PERCENT_EXP) {
        _dutyCycle[0] = maxPercent;
        _dutyCycle[1] = 1;
        _dutyCycle[2] = 1;
        _useExpStateTransition = 1;
    }

    // Rest will occur via start/stop using PIT.
    // LED cycle state machine will be handled via stateTransition() -- called by PIT interrupt
}

// Start RGB LED color cycling.
// Reenables PIT.
void LED::start() {
    DPIT::start(DPIT::PIT0);
}

// Stop RGB LED color cycling.
// Disables PIT.
void LED::stop() {
    DPIT::stop(DPIT::PIT0);
}

// Transition color cycling state.
// Should only be called by PIT interrupt handler.
void LED::stateTransition() {
    if (_useExpStateTransition) {
        return _stateTransitionPulse();
    } else {
        return _stateTransitionCycle();
    }
}

// Private API:

// LED Exponential RED Pulsing.
// To handle exponential scale, max brightness is limited to a duty cycle of 81.92% (8192/10000)
void LED::_stateTransitionPulse() {
    // State Changes: RED -> MAGENTA -> BLUE -> CYAN -> GREEN -> YELLOW -> RED
    switch (_state) {
        case RED:
            _dutyCycle[RED_IDX] = _dutyCycle[RED_IDX] >> 1; // exponential change
            DTPM::setDutyCycle(DTPM::TPM_2, DTPM::TPM_CH0, _dutyCycle[RED_IDX], DTPM::EDGE_ALIGNED);
            if (_dutyCycle[RED_IDX] == 0) {
                _dutyCycle[RED_IDX] = 1; // prep for next cycle
                _state = RED2;
            }
            break;
        case RED2:
            _dutyCycle[RED_IDX] = _dutyCycle[RED_IDX] << 1; // exponential change
            DTPM::setDutyCycle(DTPM::TPM_2, DTPM::TPM_CH0, _dutyCycle[RED_IDX], DTPM::EDGE_ALIGNED);
            if (_dutyCycle[RED_IDX] == MAX_PERCENT_EXP) {
                _state = RED;
            }
            break;
        default:
            break;
    }
}

// LED RGB Cycling (with gamma/square correction).
// Should only be called by PIT interrupt handler.
void LED::_stateTransitionCycle() {
    // State Changes: RED -> MAGENTA -> BLUE -> CYAN -> GREEN -> YELLOW -> RED
    switch (_state) {
        case RED:
            _dutyCycle[BLUE_IDX] = _dutyCycle[BLUE_IDX] + 3276;
            DTPM::setCnV(DTPM::TPM_0, DTPM::TPM_CH1, ((_dutyCycle[BLUE_IDX] * _dutyCycle[BLUE_IDX]) >> 16));
            if (_dutyCycle[BLUE_IDX] == MAX_PERCENT_SQR) {
                _state = MAGENTA;
            }
            break;
        case MAGENTA:
            _dutyCycle[RED_IDX] = _dutyCycle[RED_IDX] - 3276;
            DTPM::setCnV(DTPM::TPM_2, DTPM::TPM_CH0, ((_dutyCycle[RED_IDX] * _dutyCycle[RED_IDX]) >> 16));
            if (_dutyCycle[RED_IDX] == 0) {
                _state = BLUE;
            }
            break;
        case BLUE:
            _dutyCycle[GREEN_IDX] = _dutyCycle[GREEN_IDX] + 3276;
            DTPM::setCnV(DTPM::TPM_2, DTPM::TPM_CH1, ((_dutyCycle[GREEN_IDX] * _dutyCycle[GREEN_IDX]) >> 16));
            if (_dutyCycle[GREEN_IDX] == MAX_PERCENT_SQR) {
                _state = CYAN;
            }
            break;
        case CYAN:
            _dutyCycle[BLUE_IDX] = _dutyCycle[BLUE_IDX] - 3276;
            DTPM::setCnV(DTPM::TPM_0, DTPM::TPM_CH1, ((_dutyCycle[BLUE_IDX] * _dutyCycle[BLUE_IDX]) >> 16));
            if (_dutyCycle[BLUE_IDX] == 0) {
                _state = GREEN;
            }
            break;
        case GREEN:
            _dutyCycle[RED_IDX] = _dutyCycle[RED_IDX] + 3276;
            DTPM::setCnV(DTPM::TPM_2, DTPM::TPM_CH0, ((_dutyCycle[RED_IDX] * _dutyCycle[RED_IDX]) >> 16));
            if (_dutyCycle[RED_IDX] == MAX_PERCENT_SQR) {
                _state = YELLOW;
            }
            break;
        case YELLOW:
            _dutyCycle[GREEN_IDX] = _dutyCycle[GREEN_IDX] - 3276;
            DTPM::setCnV(DTPM::TPM_2, DTPM::TPM_CH1, ((_dutyCycle[GREEN_IDX] * _dutyCycle[GREEN_IDX]) >> 16));
            if (_dutyCycle[GREEN_IDX] == 0) {
                _state = RED;
            }
            break;
        default:
            break;
    }
}

