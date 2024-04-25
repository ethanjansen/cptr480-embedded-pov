#ifndef LED_H
#define LED_H

// LED class for managing RGB LED (notice, not calling this a driver).
// This class will initialize GPIO, TPM, and PIT using their driver classes -- global instances will be initialized in LED.cpp.
// For this lab we will color cycle the RGB LED with PWM/PIT-controlled fading.
class LED {
    public:
        // enums
        enum LEDPercents {
            MAX_PERCENT_SQR = 0xfff0,
            MAX_PERCENT_EXP = 0x2000,
        };


        // empty constructor
        LED();

        // Public API:

        // Initialize GPIO, TPM, and PIT.
        // Requires GPIO and TPM classes are configured correctly.
        // Set speed to fade. Set maxPercent to change between exponential pulsing or RGB (square) cycling.
        void init(unsigned speed, LEDPercents maxPercent);

        // Start RGB LED color cycling.
        // Reenables PIT interrupts.
        void start();

        // Stop RGB LED color cycling.
        // Disables PIT interrupts.
        void stop();

        // Transition color cycling state.
        // Should only be called by PIT interrupt handler.
        void stateTransition();

    private:
        // no copy or assignment
        LED(const LED&);
        void operator=(const LED&);

        // Private API:

        // LED Exponential RED Pulsing.
        void _stateTransitionPulse();

        // LED RGB Cycling (with gamma/square correction).
        void _stateTransitionCycle();

        // State information:

        // color shown - and what do to to get to the next state
        enum RGBState {
            RED,        // blue++
            MAGENTA,    // red--
            BLUE,       // green++
            CYAN,       // blue--
            GREEN,      // red++
            YELLOW,     // green--
            RED2, // used for pulsing
        };

        // current state
        static RGBState _state;

        enum RGBIndex {
            RED_IDX,
            GREEN_IDX,
            BLUE_IDX,
            NUM_COLORS
        };

        // duty cycle for each color
        static unsigned _dutyCycle[3]; // 0 to 10000 as per TPM implementation

        // flag for which stateTransition to use
        static unsigned _useExpStateTransition;

};

extern LED g_led; // single instance - there is only one RGB, thus using static variables

#endif // LED_H