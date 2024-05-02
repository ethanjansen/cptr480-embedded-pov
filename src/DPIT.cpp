#include "DPIT.h"
#include "DTPM.h"
#include "MKL25Z4.h"

// PIT Interval Array
unsigned DPIT::pitIntervals[NUM_PITNAMES] = {0, 0};

// empty Constructor
DPIT::DPIT() {}

// Initialize PIT clock, interrupts, and enable PIT
void DPIT::init() {
    // Enable clock to PIT
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;

    // Enable PIT interrupts in the NVIC
    NVIC_EnableIRQ(PIT_IRQn);

    // PIT master enable (MDIS=0)
    PIT->MCR &= ~PIT_MCR_MDIS_MASK;
}

// Set PIT interval (interrupts per second)
void DPIT::setInterruptsPerSec(PITName pit, unsigned interruptsPerSec) {
    // LDVAL = (bus clock / interrupts per sec) - 1
    pitIntervals[pit] = (DEFAULT_SYSTEM_CLOCK/(((SYSTEM_SIM_CLKDIV1_VALUE>>28)+1)*interruptsPerSec)) - 1;
}

// Set PIT interval (seconds per interrupt)
// Does not check for overflow!
void DPIT::setSecPerInterrupt(PITName pit, unsigned secPerInterrupt) {
    // LDVAL = (bus clock * seconds per interrrupt) - 1
    pitIntervals[pit] = (DEFAULT_SYSTEM_CLOCK/((SYSTEM_SIM_CLKDIV1_VALUE>>28)+1)*secPerInterrupt) - 1;
}

// Stops pit
void DPIT::stop(PITName pit) {
    // TCTRL CHN=0, TIE=1, TEN=1
    PIT->CHANNEL[pit].TCTRL = 0;
}

// Loads timer value and starts pit
void DPIT::start(PITName pit) {
    // Load Interrupt interval
    PIT->CHANNEL[pit].LDVAL = pitIntervals[pit];
    // Enable PIT interrupts
    // TCTRL CHN=0, TIE=1, TEN=1
    PIT->CHANNEL[pit].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
}

// PIT interrupt handler
void DPIT::IRQHandler() {
    // Check which PIT triggered the interrupt
    if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {
        // Do something - LEDBAR Custom
        stop(PIT0); // stop self
        g_tpm.stop(DTPM::TPM_2); // stop TPM to reset LEDBAR send
        start(PIT1); // start reset cycle

        // Clear flag
        PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;
    }

    if (PIT->CHANNEL[1].TFLG & PIT_TFLG_TIF_MASK) {
        // Do something - LEDBAR Custom
        stop(PIT1); // stop self
        g_tpm.start(DTPM::TPM_2); // start TPM to send LEDBAR data
        start(PIT0); // start reset cycle

        // Clear flag
        PIT->CHANNEL[1].TFLG = PIT_TFLG_TIF_MASK;
    }
}

