#include "DGPIO.h"
#include "DPIT.h"
#include "DUART.h"
#include "MKL25Z4.h"

// Create singular, global instances of the classes used in this program
DGPIO g_gpio;
DPIT g_pit;
DUART g_uart;

int main() {
    // Initialize hardware.  This is separate from the class constructors, because
    // in some applications it may be necessary to re-init (and our class instances
    // are eternal), and because the order of hardware initialization may matter,
    // so it's better to do it explicitly inside main() than implicitly at object
    // creation.
    g_gpio.Init();
    g_pit.init();
    g_uart.init();

    // Set up PIT to interrupt at 2 Hz
    g_pit.setInterruptsPerSec(DPIT::PIT0, 2);

    // Start PIT
    g_pit.start(DPIT::PIT0);

    while (1)
    {
        // mining bitcoin...
    }
} 
