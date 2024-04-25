// GPIO pin driver

#include "DGPIO.h"
#include <MKL25Z4.h>

// Master list of GPIO lines
// NOTE: order must match exactly with the GPIOName enum in DGPIO.h!
// (This is checked at runtime if assertions are enabled.)
const DGPIO::GPIOTable DGPIO::gpios[] = {
    //  name             pin    port     i/o  pu/pd  mux(alt) init
    { LED_RED,           18,  PortB, Output, Float,   3,      0 },
    { LED_GREEN,         19,  PortB, Output, Float,   3,      0 },
    { LED_BLUE,           1,  PortD, Output, Float,   4,      0 },
};


DGPIO::DGPIO()
{
}


void DGPIO::Init()
{
    PORT_Type *port;
    GPIO_Type *gpio;

    for (unsigned k = 0 ; k < sizeof(gpios)/sizeof(gpios[0]) ; k++)
    {
        //assert(k == gpios[k].name);  // GPIO table rows must be in same order as GPIO enum!

        // Enable clock for this port (bits for ports A-E are all in one group)
        if (!(SIM->SCGC5 & (SIM_SCGC5_PORTA_MASK << gpios[k].port)))
        {
            SIM->SCGC5 |= (SIM_SCGC5_PORTA_MASK << gpios[k].port);
        }

        // Calculate PORTx and GPIOx base address, for accessing PORTx_PCRy and the GPIOx registers
        // (This exploits the fact that the port A, B, C, etc. register address ranges are consecutive,
        // with predictable spacing---see the reference manual.)
        port = (PORT_Type *)(PORTA_BASE + gpios[k].port * 0x1000);
        gpio = (GPIO_Type *)(GPIOA_BASE + gpios[k].port * 0x0040);

        // Set alternate function to mux. Default should be 1 (GPIO)
        port->PCR[gpios[k].pin] = (port->PCR[gpios[k].pin] & ~PORT_PCR_MUX_MASK) | PORT_PCR_MUX(gpios[k].mux);

        // Enable pull-up or pull-down resistor, or neither
        if (gpios[k].pupd == Float)
        {   // no pull-up/down: PE=0
            port->PCR[gpios[k].pin] &= (~PORT_PCR_PE_MASK);
        }
        else
        {   // either pull-up or pull-down: PE=1
            port->PCR[gpios[k].pin] |= PORT_PCR_PE_MASK;
            if (gpios[k].pupd == PD)
            {   // pull-down: PS=0
                port->PCR[gpios[k].pin] &= (~PORT_PCR_PS_MASK);
            }
            else
            {   // pull-up: PS=1
                port->PCR[gpios[k].pin] |= PORT_PCR_PS_MASK;
            }
        }

        // Everything that depends on input vs output
        if (gpios[k].io == Output)
        {
            // Initial value
            if (gpios[k].init)
            {   // initial output state is high (1)
                gpio->PSOR = (1 << gpios[k].pin);
            }
            else
            {   // initial output state is low (0)
                gpio->PCOR = (1 << gpios[k].pin);
            }
            // Set direction to output
            gpio->PDDR |= (1 << gpios[k].pin);
        }
        else
        {
            // Set direction to input
            gpio->PDDR &= ~(1 << gpios[k].pin);
        }
    }
}


// Read the named GPIO and return true if it is a 1, false if 0.
bool DGPIO::Status(GPIOName name)
{
    GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + gpios[name].port * 0x0040);
    return (gpio->PDIR & (1 << gpios[name].pin)) ? true : false;
}


// Set the named GPIO.
void DGPIO::Set(GPIOName name)
{
    GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + gpios[name].port * 0x0040);
    gpio->PSOR = (1 << gpios[name].pin);
}


// Clear the named GPIO.
void DGPIO::Clear(GPIOName name)
{
    GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + gpios[name].port * 0x0040);
    gpio->PCOR = (1 << gpios[name].pin);
}


// Toggle the named GPIO.
void DGPIO::Toggle(GPIOName name)
{
    GPIO_Type *gpio = (GPIO_Type *)(GPIOA_BASE + gpios[name].port * 0x0040);
    gpio->PTOR = (1 << gpios[name].pin);
}
