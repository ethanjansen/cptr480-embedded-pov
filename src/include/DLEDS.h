#ifndef DLEDS_H
#define DLEDS_H

#include "DGPIO.h" // for getDMADst()
#include <stdint.h>

class DLEDS
{
public:
    enum
    {
        NUM_STRINGS = 8,
        NUM_LEDS_PER_STRING = 16,
        NUM_LEDS = NUM_STRINGS * NUM_LEDS_PER_STRING,

        //NUM_FRAMES = 120, //rgb_data
        NUM_FRAMES = 150, //rgb_data2

        RESET_FREQUENCY = 333, // in Hz
        RESET_PERIOD = 1000/RESET_FREQUENCY, // in ms
    };
    
    // Public API
    static void init();

    // Get initialization status
    static inline bool isInit() { return _init; }

    // Permute RGB data into the form needed for PTC0-7 outputs
    static void swizzle(const uint16_t R[], const uint16_t G[], const uint16_t B[], int os, bool inv);

    // Get buffer pointer
    static inline const uint8_t *getDMABuf() { return _dmaBuf; }

    // Get DMA destination pointer
    static inline void *getDMADest() { return DGPIO::getOutputRegister(DGPIO::LEDS_SEG0); }

    // PIT interrupt handler
    // Ends LED Reset Period
    // Called by PIT handler
    static void stopLEDReset();

    // DMA interrupt handler
    // Starts LED Reset Period
    // Called by DMA handler
    static void startLEDReset();

private:
    // Private Data

    // RAM buffer to be sent out PTC0-7 using DMA
    static uint8_t _dmaBuf[768];

    // RGB Data
    static const uint16_t _R[NUM_FRAMES][NUM_LEDS], _G[NUM_FRAMES][NUM_LEDS], _B[NUM_FRAMES][NUM_LEDS];

    // frame and if we should reverse
    static int _frame;
    static bool _reverse;

    // gyro data
    static signed short _gyroZ;

    // saved angle (2^16 theta)
    static signed int _angle;

    // init status
    static bool _init;

    // Private API

    // swizzle helper functions
    static void transpose8rS32(uint32_t *x, uint32_t *y);
    static void setPartLow(uint32_t *x, uint32_t first, uint32_t second, uint32_t third, uint32_t fourth, bool inv);
    static void setPartHigh(uint32_t *x, uint32_t first, uint32_t second, uint32_t third, uint32_t fourth, bool inv);

    // Frame helper function
    static void getFrame();

    // no copy or assignment or constructor
    DLEDS();
    DLEDS(const DLEDS&);
    void operator=(const DLEDS&);
};

#endif // DLEDS_H
