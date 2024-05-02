// GPIO pin driver
#include "DGPIO.h"
#include "MKL25Z4.h"

// Other classes for interruptHandlers
#include "DROTENC.h"

// Master list of GPIO lines
// NOTE: order must match exactly with the GPIOName enum in DGPIO.h!
// (This is checked at runtime if assertions are enabled.)
const DGPIO::GPIOTable DGPIO::gpios[] = {
    //  name        pin    port     i/o  pu/pd  interrupt        mux(alt)   init  interruptHandler
    { LED_RED,       18,  PortB, Output, Float, Disabled,            1,       1,   nullptr                    },  // Big RGB LED on the FRDM-KL25Z board -- active low // use alt 3 for PWM 
    { LED_GREEN,     19,  PortB, Output, Float, Disabled,            1,       1,   nullptr                    },  // use alt 3 for PWM
    { LED_BLUE,       1,  PortD, Output, Float, Disabled,            1,       1,   nullptr                    },  // use alt 4 for PWM
    { ENCODER1,       4,  PortA, Input,  Float, INT_BothEdges,       1,       0,   DROTENC::controlPosition   },  // Rotary encoder pins have their own pull-ups on the PCB
    { ENCODER2,       5,  PortA, Input,  Float, INT_BothEdges,       1,       0,   DROTENC::controlPosition   },  
    { SW2,            0,  PortD, Input,  PU,    INT_RisingEdge,      1,       0,   DROTENC::resetPosition     },  // Push-button switches; internal pull-ups are needed
    { SW3,            0,  PortB, Input,  PU,    Disabled,            1,       0,   nullptr                    },
    { UART0_RX,       1,  PortA, Input,  Float, Disabled,            2,       0,   nullptr                    }, // UART0
    { UART0_TX,       2,  PortA, Output, Float, Disabled,            2,       0,   nullptr                    },
    { LEDBAR_LOW,     2,  PortB, Output, Float, Disabled,            3,       0,   nullptr                    },  // LED bar configuration // use alt 3 for PWM
    { LEDBAR_HIGH,    3,  PortB, Output, Float, Disabled,            3,       0,   nullptr                    },  // use alt 3 for PWM
    { LEDBAR_SEG0,    0,  PortC, Output, Float, Disabled,            1,       0,   nullptr                    },
    { LEDBAR_SEG1,    1,  PortC, Output, Float, Disabled,            1,       0,   nullptr                    },
    { LEDBAR_SEG2,    2,  PortC, Output, Float, Disabled,            1,       0,   nullptr                    },
    { LEDBAR_SEG3,    3,  PortC, Output, Float, Disabled,            1,       0,   nullptr                    },
    { LEDBAR_SEG4,    4,  PortC, Output, Float, Disabled,            1,       0,   nullptr                    },
    { LEDBAR_SEG5,    5,  PortC, Output, Float, Disabled,            1,       0,   nullptr                    },
    { LEDBAR_SEG6,    6,  PortC, Output, Float, Disabled,            1,       0,   nullptr                    },
    { LEDBAR_SEG7,    7,  PortC, Output, Float, Disabled,            1,       0,   nullptr                    },
};

// Private instance of _interruptableGpios[]
unsigned DGPIO::_interruptableGpioIndecies[NUM_GPIONAMES]; // set at runtime
unsigned DGPIO::_numInterruptableGpioIndecies = 0;


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

        // Set alternate function to mux. Default should be 1 (GPIO), so clear first
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

        // Set pin interrrupt if available. Default should be 0 (Disabled).
        // Do this Last!!! This will prevent any interrupts from happening early (which could cause HardFault)
        if (gpios[k].interrupt && (gpios[k].port == PortA || gpios[k].port == PortD))
        {
            // enable interrupts
            port->ISFR = 0xFFFFFFFF;
            port->PCR[gpios[k].pin] |= PORT_PCR_IRQC(gpios[k].interrupt);
            NVIC_EnableIRQ((IRQn_Type)(PORTA_IRQn + (gpios[k].port/PortD))); // magic: port==0 for A so use PORTA_IRQn, port==3 for D so use PORTA_IRQn+1==PORTD_IRQn

            // record
            _interruptableGpioIndecies[_numInterruptableGpioIndecies++] = k;
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


// Interrupt handler for GPIO ports A and D
void DGPIO::IRQHandler()
{
    // iterate through all interruptable GPIOs
    for (unsigned i=0; i<_numInterruptableGpioIndecies; i++)
    {
        unsigned k = _interruptableGpioIndecies[i];
        // check correct port
        switch (gpios[k].port) {
            case PortA:
                if (PORTA->PCR[gpios[k].pin] & PORT_PCR_ISF_MASK)
                {
                    if (gpios[k].handler)
                    {
                        // handle interrupt
                        gpios[k].handler();
                    }     
                }
                break;
            case PortD:
                if (PORTD->PCR[gpios[k].pin] & PORT_PCR_ISF_MASK)
                {
                    if (gpios[k].handler)
                    {
                        // handle interrupt
                        gpios[k].handler();
                    } 
                }
                break;
            default:
                // error
                break;
        }
    }

    // clear all interrupt flags
    PORTA->ISFR = 0xFFFFFFFF;
    PORTD->ISFR = 0xFFFFFFFF;
}