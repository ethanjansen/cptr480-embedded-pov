#ifndef INT2STR_H
#define INT2STR_H

// Based on https://stackoverflow.com/questions/69779184/printing-number-as-string-in-c-without-stdlibs
// This supports base 2 through base 16.
//
// Modified by Ethan Jansen 4/30/2024
//
// NOTE: If you need a fancier replacement for printf, you might consider something like
// https://github.com/mpaland/printf instead.  This is just an example of a simple number-to-string
// conversion.
void int2str(int i, int base, char *str, int maxLen);


#endif // INT2STR_H