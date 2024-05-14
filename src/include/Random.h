#ifndef RANDOM_H
#define RANDOM_H

// Pseudorandom number generator
// No initialization is required, but you can set the seed with Random::seed(unsigned)
class Random {
    public:
        // Set seed
        static void seed(unsigned s);

        // Get random number (0 to uint32 max value)
        static unsigned rand();

        // Get random number between min and max
        static unsigned randBetween(unsigned min, unsigned max);

    private:
        // no copy, assignment, or constructor
        Random();
        Random(const Random&);
        void operator=(const Random&);

        // private data
        static unsigned _seed;

        // splitmix32 pseudorandom number generator
        static unsigned _splitmix32();
};


#endif // RANDOM_H