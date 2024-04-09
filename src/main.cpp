#include "DGPIO.h"
#include "DPIT.h"
#include "Morse.h"
#include "MKL25Z4.h"

// Global Constants
const char* const morseMessage = "SOS";

// Create singular, global instances of the classes used in this program
DGPIO g_gpio;
DPIT g_pit;
Morse g_morse;

int main() {
    // Initialize hardware.  This is separate from the class constructors, because
    // in some applications it may be necessary to re-init (and our class instances
    // are eternal), and because the order of hardware initialization may matter,
    // so it's better to do it explicitly inside main() than implicitly at object
    // creation.
    g_gpio.Init();

    // Initialize morse code
    g_morse.getMorseString(morseMessage);

    // Initialize PIT
    g_pit.init();

    // Set up PIT to interrupt at 1Hz
    g_pit.setInterruptsPerSec(DPIT::PIT0, 1);

    // Start PIT
    g_pit.start(DPIT::PIT0);

    while (1)
    {
        // I dunno... mine some bitcoin or something?
    }
} 
