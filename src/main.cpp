#include "DMOTION.h"
#include "DPIT.h"
#include "MKL25Z4.h"

int main() {
    // Delay startup by ~15ms using PIT
    DPIT::init();
    DPIT::sleep(DPIT::PIT1, 15);

    // Let DMOTION class initialize hardware.
    DMOTION::init();

    while (1)
    {
        // mining bitcoin...
    }
} 


