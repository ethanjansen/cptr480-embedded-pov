#include "Random.h"

// static varaiable initialization
unsigned Random::_seed = 0xdeadbeef; // default seed

// Set seed
void Random::seed(unsigned s) {
    _seed = s;
}

// Get random number (0 to uint32 max value)
unsigned Random::rand() {
    return _splitmix32();
}

// Get random number between min and max
// Distribution is *not* uniform
unsigned Random::randBetween(unsigned min, unsigned max) {
    return min + _splitmix32() % (max - min);
}

// Private pseudorandom number generator

// splitmix32 pseudorandom number generator
// https://stackoverflow.com/a/52056161
unsigned Random::_splitmix32() {
    unsigned z = (_seed += 0x9e3779b9);
    z ^= z >> 16;
    z *= 0x21f0aaad;
    z ^= z >> 15;
    z *= 0x735a2d97;
    z ^= z >> 15;
    return z;
}
