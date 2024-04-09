#ifndef MORSE_H
#define MORSE_H

class Morse {
    public:
        // character size limit of Morse string buffer
        static const unsigned MAX_MORSE_STRING_SIZE = 300;

        // Morse Characters: 0=low, 1=high
        // dit: "dit duration" is one time unit long
        // dah: three time units long
        // inter-element gap: one time unit long
        // short gap (between letters): three time units long
        // medium gap (between words): seven time units long (space will come after letter gap, so only need 4 zeros)
        // TODO: Not implementing punctuation for now
        struct MorseChar {
            unsigned length : 5;
            unsigned code : 22; // longest is 0: 5 dahs (15) + 4 inter-element gaps (4) + 1 short gap (3) = 22
        };

        // empty constructor
        Morse();

        // Get Morse code for a character
        static MorseChar getMorseChar(char c);

        // Get Morse code for a string
        // Saves to morseString
        static void getMorseString(const char *str);

        // Clear Morse string
        static void clearMorseString();

        // Read next Morse string bit
        static unsigned readNextMorseStringBit();
    
    private:
        // no copy or assignment
        Morse(const Morse&);
        void operator=(const Morse&);

        // Morse LUT
        // align with ascii
        static const MorseChar morseLUT[38];

        // Morse String to Output
        static MorseChar morseString[MAX_MORSE_STRING_SIZE];

        // location to read from
        static unsigned arrayIndex;
        static unsigned bitIndex;        

};

// single global instance
extern Morse g_morse;

#endif