#include "DMOTION.h"
#include "DPIT.h"
#include "MKL25Z4.h"

int main() {
    // Delay startup by ~15ms using PIT
    DPIT::init();
    DPIT::sleep(DPIT::PIT1, 15);

    // Let DMOTION class initialize hardware.
    DMOTION::init();

    // Probe for LSM6DSL
    bool exists = DMOTION::probe(); // I will check this with debugger

    while (1)
    {
        // mining bitcoin...
    }
} 


