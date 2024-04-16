#include "DPIT.h"
#include "DGPIO.h"
#include "DUART.h"
#include <MKL25Z4.h>

// Global variables
static const char *UART_MESSAGE = "Hello, World!\n";
volatile unsigned g_ledStatus = 0;

// PIT Interval Array
unsigned DPIT::pitIntervals[NUM_PITNAMES] = {0, 0};

// empty Constructor
DPIT::DPIT() {}

// Initialize PIT clock, interrupts, and enable PIT
void DPIT::init() {
    // TODO: move stuff to start. Also support multiple pits.

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
    // Clear interrupt flag
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;

    // toggle LED - active low
    g_ledStatus = !g_ledStatus;
    if (g_ledStatus) {
        g_gpio.Clear(DGPIO::LED_GREEN);
    } else {
        g_gpio.Set(DGPIO::LED_GREEN);
    }

    // Output message over UART
    if(g_uart.sendString(UART_MESSAGE)) {
        // buffer is full, indicate with RED LED
        // I tested this by using a BAUD of 300 and a frequency >20Hz
        g_gpio.Clear(DGPIO::LED_RED);
    } else {
        g_gpio.Set(DGPIO::LED_RED);
    }

}

