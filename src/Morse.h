#ifndef MORSE_H
#define MORSE_H

class Morse {
    public:
        // empty constructor
        Morse();
    private:
        // no copy or assignment
        Morse(const Morse&);
        void operator=(const Morse&);

        // Morse Characters: 0=low, 1=high
        // dit: "dit duration" is one time unit long
        // dah: three time units long
        // inter-element gap: one time unit long
        // short gap (between letters): three time units long
        // medium gap (between words): seven time units long (space will come after letter gap, so only need 4 zeros)
        // TODO: Not implementing punctuation for now
        struct MorseChar {
            unsigned length : 3;
            unsigned code : 22; // longest is 0: 5 dahs (15) + 4 inter-element gaps (4) + 1 short gap (3) = 22
        };

        // Morse LUT
        // align with ascii
        static const MorseChar morseLUT[37];

};

// single global instance
extern Morse g_morse;

#endif