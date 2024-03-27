#include "DGPIO.h"
#include "MKL25Z4.h"

// Create singular, global instances of the classes used in this program
DGPIO g_gpio;


int main() {
    // Initialize hardware.  This is separate from the class constructors, because
    // in some applications it may be necessary to re-init (and our class instances
    // are eternal), and because the order of hardware initialization may matter,
    // so it's better to do it explicitly inside main() than implicitly at object
    // creation.
    g_gpio.Init();

    // Set up periodic interrupts at 4 Hz (it might be better to have a PIT driver class
    // but this is just a dirty demo).
    //
    // Enable clock to PIT
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;

    // Enable PIT interrupts in the NVIC
    NVIC_EnableIRQ(PIT_IRQn);

    // PIT master enable (MDIS=0)
    PIT->MCR &= ~PIT_MCR_MDIS_MASK;
    // Set PIT channel 0 for 4 Hz interrupts, and enable
    // LDVAL = (bus clock / 4) - 1 = 24e6/4 - 1 = 5999999
    // TCTRL CHN=0, TIE=1, TEN=1
    PIT->CHANNEL[0].LDVAL = 5999999;
    PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;


    while (1)
    {
        // I dunno... mine some bitcoin or something?
    }
} 
