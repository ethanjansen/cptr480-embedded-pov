// C-linkage interrupt-handler stubs to call the actual handlers inside C++ classes

#include "DGPIO.h"
#include "MKL25Z4.h"
#include <stdint.h>

volatile uint32_t g_sysTicks = 0;
volatile uint32_t g_pitTicks = 0;


extern "C" {

void SysTick_Handler()
{
    g_sysTicks++;
}


void PIT_IRQHandler()
{
    // Clear interrupt flag
    PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK;

    g_pitTicks++;

    if (g_pitTicks & 0x01)
        g_gpio.Clear(DGPIO::LED_RED);
    else
        g_gpio.Set(DGPIO::LED_RED);

    if (g_pitTicks & 0x08)
        g_gpio.Clear(DGPIO::LED_BLUE);
    else
        g_gpio.Set(DGPIO::LED_BLUE);
     
    if (g_pitTicks & 0x10)
        g_gpio.Clear(DGPIO::LED_GREEN);
    else
        g_gpio.Set(DGPIO::LED_GREEN);
}

} // extern "C"
