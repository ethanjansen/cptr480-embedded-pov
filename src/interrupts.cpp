// C-linkage interrupt-handler stubs to call the actual handlers inside C++ classes
#include "DGPIO.h"
#include "DPIT.h"
#include <stdint.h>

volatile uint32_t g_sysTicks = 0;

extern "C" {

void SysTick_Handler()
{
    g_sysTicks++;
}


void PIT_IRQHandler()
{
    // Call the PIT interrupt handler
    g_pit.IRQHandler();
}

} // extern "C"
