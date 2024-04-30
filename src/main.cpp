#include "LED.h"
#include "MKL25Z4.h"
#include <cstdlib>
#include <cstring>

// Local function prototypes
void int2str(int i, int base, char *str, int maxLen);


// Create singular, global instances of the classes used in this program
LED g_led;

int main() {
    // Let LED class initialize hardware.
    // g_led.init(15, LED::MAX_PERCENT_EXP); // Pulses RED
    g_led.init(17, LED::MAX_PERCENT_SQR);

    // Start LED color cycling.
    g_led.start();

    while (1)
    {
        // mining bitcoin...
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