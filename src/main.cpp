#include "LED.h"
#include "MKL25Z4.h"

// Create singular, global instances of the classes used in this program
LED g_led;

int main() {
    // Let LED class initialize hardware.
    // g_led.init(15, LED::MAX_PERCENT_EXP); // Pulses RED
    g_led.init(17, LED::MAX_PERCENT_SQR);

    // Start LED color cycling.
    g_led.start();

    while (1)
    {
        // mining bitcoin...
    }
} 
