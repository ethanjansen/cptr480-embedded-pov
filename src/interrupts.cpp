// C-linkage interrupt-handler stubs to call the actual handlers inside C++ classes
#include "DGPIO.h"
#include "DSPI.h"
#include "DPIT.h"
#include <stdint.h>

volatile uint32_t g_sysTicks = 0;

extern "C" {

void SysTick_Handler()
{
    g_sysTicks++;
}


void PORTA_IRQHandler()
{
    // Call the GPIO interrupt handler
    DGPIO::IRQHandler();
}


void PORTD_IRQHandler()
{
    // Call the GPIO interrupt handler
    DGPIO::IRQHandler();
}


void PIT_IRQHandler()
{
    // Call the PIT interrupt handler
    DPIT::IRQHandler();
}

/* // not used in this lab
void UART0_IRQHandler() {
    // Call the UART0 interrupt handler
    DUART::IRQHandler();
}


void TPM0_IRQHandler() {
    // Call the TPM0 interrupt handler
    DTPM::IRQHandler();
}


void TPM1_IRQHandler() {
    // Call the TPM1 interrupt handler
    DTPM::IRQHandler();
}


void TPM2_IRQHandler() {
    // Call the TPM2 interrupt handler
    DTPM::IRQHandler();
}
*/

void SPI0_IRQHandler() {
    // call the SPI interrupt handler
    DSPI::IRQHandler();
}


void SPI1_IRQHandler() {
    // call the SPI interrupt handler
    DSPI::IRQHandler();
}

} // extern "C"
