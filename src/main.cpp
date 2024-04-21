#include "DGPIO.h"
#include "MKL25Z4.h"
#include <cstdlib>
#include <cstring>

// Local function prototypes
void int2str(int i, int base, char *str, int maxLen);


// Create singular, global instances of the classes used in this program
DGPIO g_gpio;


int main() {
    // Initialize hardware.  This is separate from the class constructors, because
    // in some applications it may be necessary to re-init (and our class instances
    // are eternal), and because the order of hardware initialization may matter,
    // so it's better to do it explicitly inside main() than implicitly at object
    // creation.
    g_gpio.Init();

    // Set up periodic interrupts at 4 Hz (it might be better to have a PIT driver class
    // but this is just a dirty demo).
    //
    // Enable clock to PIT
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;

    // Enable PIT interrupts in the NVIC
    NVIC_EnableIRQ(PIT_IRQn);

    // PIT master enable (MDIS=0)
    PIT->MCR &= ~PIT_MCR_MDIS_MASK;
    // Set PIT channel 0 for 4 Hz interrupts, and enable
    // LDVAL = (bus clock / 4) - 1 = 24e6/4 - 1 = 5999999
    // TCTRL CHN=0, TIE=1, TEN=1
    PIT->CHANNEL[0].LDVAL = 5999999;
    PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;


    while (1)
    {
        // I dunno... mine some bitcoin or something?
    }
} 


// Based on https://stackoverflow.com/questions/69779184/printing-number-as-string-in-c-without-stdlibs
// This supports base 2 through base 16.
//
// NOTE: If you need a fancier replacement for printf, you might consider something like
// https://github.com/mpaland/printf instead.  This is just an example of a simple number-to-string
// conversion.
void int2str(int i, int base, char *str, int maxLen)
{
    int p = maxLen - 1; // Start from the "right".

    int value = i < 0 ? i : -i;  // negative absolute value
    do
    {
        div_t qr = div(value, base);
        str[p--] = "0123456789abcdef"[-qr.rem];  // Table look up
        value = qr.quot;
    }
    while (value && p >= 0);

    if (value)
    {   // Fill with '#' symbols when value is too big to display.
        memset(str, '#', maxLen);
    }
    else
    {
        if (i < 0)
        {
            str[p--] = '-';
        }
        // Pad left side with spaces.
        memset(str, ' ', p+1);
    }
}