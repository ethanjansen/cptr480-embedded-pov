#include "int2str.h"

// Based on https://stackoverflow.com/questions/69779184/printing-number-as-string-in-c-without-stdlibs
// This supports base 2 through base 16.
//
// NOTE: If you need a fancier replacement for printf, you might consider something like
// https://github.com/mpaland/printf instead.  This is just an example of a simple number-to-string
// conversion.
void int2str(int i, int base, char *str, int maxLen)
{
    // string index
    int p = maxLen - 1; // Start from the "right".
    str[p] = '\0';  // Null terminator

    int value = i < 0 ? -i : i;  // absolute value
    do
    {
        str[--p] = "0123456789abcdef"[value % base];  // Table look up
        value = value / base;
    }
    while (value && p > 0);

    if (value || (p == 0 && i < 0)) // check if no room for negative sign
    {   // Fill with '#' symbols when value is too big to display. -- leave null terminator
        for (int j = 0; j < maxLen; j++)
        {
            str[j] = '#';
        }
    }
    else if (i < 0)
    {
        str[--p] = '-';
    }

    // pad left with spaces
    while (p > 0)
    {
        str[--p] = ' ';
    }
}