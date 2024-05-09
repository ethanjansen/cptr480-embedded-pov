#include "DPIT.h"
#include "MKL25Z4.h"

// PIT Interval Array
unsigned DPIT::pitIntervals[NUM_PITNAMES] = {0, 0};

// static instantiation
bool DPIT::_init;
bool DPIT::_block[NUM_PITNAMES];

// Initialize PIT clock, interrupts, and enable PIT
void DPIT::init() {
    // Enable clock to PIT
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;

    // Enable PIT interrupts in the NVIC
    NVIC_EnableIRQ(PIT_IRQn);

    // PIT master enable (MDIS=0)
    PIT->MCR &= ~PIT_MCR_MDIS_MASK;

    _init = true;
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

// Partially deterministic sleep.
// automatically sets "pit" PIT interval based on "ms", starts and stops "pit".
// This is blocking.
// Does not check for overflow!
void DPIT::sleep(PITName pit, unsigned ms) {
    // Set PIT interval
    pitIntervals[pit] = (DEFAULT_SYSTEM_CLOCK/(((SYSTEM_SIM_CLKDIV1_VALUE>>28)+1)*1000)*ms) - 1;
    // setup block
    _block[pit] = true;
    // Start PIT
    start(pit);
    // block
    while (_block[pit]) {}
    // Stop PIT
    stop(pit);
}

// PIT interrupt handler
void DPIT::IRQHandler() {
    for (unsigned i=0; i<NUM_PITNAMES; i++) {
        if (PIT->CHANNEL[i].TFLG & PIT_TFLG_TIF_MASK) {
            // Stop blocking
            _block[i] = false;
            // Clear interrupt flag
            PIT->CHANNEL[i].TFLG = PIT_TFLG_TIF_MASK;
        }
    }
}

