// C-linkage interrupt-handler stubs to call the actual handlers inside C++ classes
#include "DGPIO.h"
#include "DPIT.h"
#include "DTPM.h"
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
    g_gpio.IRQHandler();
}

void PORTD_IRQHandler()
{
    // Call the GPIO interrupt handler
    g_gpio.IRQHandler();
}

void PIT_IRQHandler()
{
    // Call the PIT interrupt handler
    g_pit.IRQHandler();
}

/* // Unused for Lab 03
void UART0_IRQHandler() {
    // Call the UART0 interrupt handler
    g_uart.IRQHandler();
}
*/

void TPM0_IRQHandler() {
    // Call the TPM0 interrupt handler
    g_tpm.IRQHandler();
}


void TPM1_IRQHandler() {
    // Call the TPM1 interrupt handler
    g_tpm.IRQHandler();
}


void TPM2_IRQHandler() {
    // Call the TPM2 interrupt handler
    g_tpm.IRQHandler();
}

} // extern "C"
