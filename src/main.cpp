#include "DGPIO.h"
#include <MKL25Z4.h>
#include <cstdio>

DGPIO g_gpio;

unsigned Iter = 0;

int main() {
    g_gpio.Init();

    PORTB->PCR[19] = PORT_PCR_MUX(1);
    PORTB->PCR[18] = PORT_PCR_MUX(1);
    PORTD->PCR[1]  = PORT_PCR_MUX(1);
    GPIOB->PDDR |= (1 << 19);
    GPIOB->PDDR |= (1 << 18);
    GPIOD->PDDR |= (1 << 1);

    GPIOB->PDOR = (GPIOB->PDOR & 0xfff3ffff) | 0x000c0000;
    GPIOD->PDOR = (GPIOB->PDOR & 0xfffffffc) | 0x00000003;

    while (1)
    {
        //GPIOB->PTOR |= (1 << 18);
        //GPIOB->PTOR |= (1 << 19);
        g_gpio.Toggle(DGPIO::LED_BLUE);

        for (int a = 0 ; a < 100000 ; a++)
        {}

        printf("Test %d\n", Iter++);
    }
} 
