#include "Morse.h"

// Private variables
const Morse::MorseChar Morse::morseLUT[38] = {
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
    {6, 0b111000},                  // S
    {8, 0b10101000},                // T
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
    case ' ':
        return morseLUT[0];
    case '0' ... '9':
        return morseLUT[c - '0' + 1];
    case 'A' ... 'Z':
        return morseLUT[c - 'A' + 11];
    case 'a' ... 'z':
        return morseLUT[c - 'a' + 11];
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
        Morse::morseString[i] = getMorseChar(str[i]);
        i++;
    }
}

// Clear Morse String
void Morse::clearMorseString()
{
    Morse::arrayIndex = 0;
    Morse::bitIndex = 0;
}

// Read next Morse string bit
unsigned Morse::readNextMorseStringBit()
{
    // if outside of MorseString bounds
    if (Morse::arrayIndex >= Morse::MAX_MORSE_STRING_SIZE)
    {
        return 0;
    }

    // get MorseChar
    Morse::MorseChar mc = Morse::morseString[Morse::arrayIndex];

    // check if end of Morse String
    if (mc.length == 0)
    {
        return 0;
    }

    // get bit
    unsigned bit = (mc.code >> (mc.length - Morse::bitIndex - 1)) & 0x01;

    // increment bitIndex and check if need to start new MorseChar
    Morse::bitIndex++;
    if (Morse::bitIndex == mc.length)
    {
        Morse::arrayIndex++;
        Morse::bitIndex = 0;
    }

    return bit;
}