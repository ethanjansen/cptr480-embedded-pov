#include "Morse.h"

// Private variables
const Morse::MorseChar Morse::_morseLUT[38] = {
    {0, 0},                         // END
    {4, 0},                         // space // 32
    {22, 0b1110111011101110111000}, // 0 // 48
    {17, 0b10111011101110111},      // 1
    {18, 0b101011101110111000},     // 2
    {16, 0b1010101110111000},       // 3
    {14, 0b10101010111000},         // 4
    {12, 0b101010101000},           // 5
    {14, 0b11101010101000},         // 6
    {16, 0b1110111010101000},       // 7
    {18, 0b111011101110101000},     // 8
    {20, 0b11101110111011101000},   // 9 // 57
    {8, 0b10111000},                // A // 65 and 97
    {12, 0b111010101000},           // B
    {14, 0b11101011101000},         // C
    {10, 0b1110101000},             // D
    {4, 0b1000},                    // E
    {12, 0b101011101000},           // F
    {12, 0b111011101000},           // G
    {10, 0b1010101000},             // H
    {6, 0b101000},                  // I
    {16, 0b1011101110111000},       // J
    {12, 0b111010111000},           // K
    {12, 0b101110101000},           // L
    {10, 0b1110111000},             // M
    {8, 0b11101000},                // N
    {14, 0b11101110111000},         // O
    {14, 0b10111011101000},         // P
    {16, 0b1110111010111000},       // Q
    {10, 0b1011101000},             // R
    {8, 0b10101000},                // S
    {6, 0b111000},                  // T
    {10, 0b1010111000},             // U
    {12, 0b101010111000},           // V
    {12, 0b101110111000},           // W
    {14, 0b11101010111000},         // X
    {16, 0b1110101110111000},       // Y
    {14, 0b11101110101000},         // Z // 90 and 122
};

// empty constructor
Morse::Morse() {}

// Get Morse code for a character
Morse::MorseChar Morse::getMorseChar(char c)
{
    switch (c)
    {
    case '\0':
        return _morseLUT[0];
    case ' ':
        return _morseLUT[1];
    case '0' ... '9':
        return _morseLUT[c - '0' + 2];
    case 'A' ... 'Z':
        return _morseLUT[c - 'A' + 12];
    case 'a' ... 'z':
        return _morseLUT[c - 'a' + 12];
    default:
        // shouldn't have gotten here
        // assume a space
        return _morseLUT[1];
    }
}

// Get Morse code for a string
// Saves to morseString
void Morse::getMorseString(const char *str)
{
    Morse::clearMorseString();

    // Add Morse characters to MorseString
    unsigned i = 0;
    while (str[i] != '\0' && i < Morse::MAX_MORSE_STRING_SIZE)
    {
        Morse::_morseString[i] = getMorseChar(str[i]);
        i++;
    }

    // Add end of MorseString
    Morse::_morseString[i] = getMorseChar('\0');
}

// Clear Morse String
void Morse::clearMorseString()
{
    Morse::_arrayIndex = 0;
    Morse::_bitIndex = 0;
}

// Read next Morse string bit
unsigned Morse::readNextMorseStringBit()
{
    // if outside of MorseString bounds
    if (Morse::_arrayIndex >= Morse::MAX_MORSE_STRING_SIZE)
    {
        return 0;
    }

    // get MorseChar
    Morse::MorseChar mc = Morse::_morseString[Morse::_arrayIndex];

    // check if end of Morse String
    if (mc.length == 0)
    {
        return 0;
    }

    // get bit
    unsigned bit = (mc.code >> (mc.length - Morse::_bitIndex - 1)) & 0x01;

    // increment bitIndex and check if need to start new MorseChar
    Morse::_bitIndex++;
    if (Morse::_bitIndex == mc.length)
    {
        Morse::_arrayIndex++;
        Morse::_bitIndex = 0;
    }

    return bit;
}