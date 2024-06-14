// WS2816 RGB LED driver
#include "DLEDS.h"
#include "MKL25Z4.h"
// Drivers to initialize/setup
#include "DDMA.h"
#include "DGPIO.h"
#include "DTPM.h"
#include "DPIT.h"
#include "DMOTION.h"


// Static data
int DLEDS::_frame;
bool DLEDS::_reverse;
signed short DLEDS::_gyroZ;
signed int DLEDS::_angle; // 2^16 theta using 500dps
bool DLEDS::_init;
uint8_t DLEDS::_dmaBuf[768];


// Initialize LEDS Display
void DLEDS::init() {
    // Initialize data
    swizzle(_R[0], _G[0], _B[0], 0, false);
    
    // Initialize GPIO, TPM, MOTION, and PIT.
    if (!DGPIO::isInit())
      DGPIO::init();
    if (!DTPM::isInit())
      DTPM::init();
    if (!DMOTION::isInit())
      DMOTION::init();
    if (!DPIT::isInit())
      DPIT::init();

    // Set Interrupt Priorities
    NVIC_SetPriority(SPI1_IRQn, 0);
    NVIC_SetPriority(DMA0_IRQn, 1);
    NVIC_SetPriority(PIT_IRQn, 2);

    // Setup PIT for reset time
    DPIT::setInterruptsPerSec(DPIT::PIT0, RESET_FREQUENCY); // 100 seems to work okay (ideally 3334 for ~300us)
    DPIT::setInterruptHandler(DPIT::PIT0, DLEDS::stopLEDReset);

    // Initialize DMA (last)
    if (!DDMA::isInit())
      DDMA::init();

    _init = true;
}


// LED Reset
void DLEDS::startLEDReset() {
    // test
    //DGPIO::Toggle(DGPIO::TEST);
    // stop DMA
    DDMA::stop(DDMA::DMA_CHANNEL_0);
    // stop TPM
    DTPM::stop(DTPM::TPM_2);
    // start PIT
    DPIT::start(DPIT::PIT0);
    // Get data
    DMOTION::getGyroZFromSPI(true); // blocking
    DMOTION::getGyroZ(&_gyroZ);
    // Update angle and Get Frame
    DLEDS::getFrame();
    // swizzle data
    DLEDS::swizzle(_R[_frame], _G[_frame], _B[_frame], 0, _reverse);
    // test
    //DGPIO::Toggle(DGPIO::TEST);
}


void DLEDS::stopLEDReset() {
    // stop PIT
    DPIT::stop(DPIT::PIT0);
    // start DMA
    DDMA::start(DDMA::DMA_CHANNEL_0);
    // startTPM
    DTPM::start(DTPM::TPM_2);
}


// Get Frame
void DLEDS::getFrame() {
  // Update angle
  if (_gyroZ <= 0) {
    _angle += (1+RESET_PERIOD)*_gyroZ;
  } else {
    _angle += (1+RESET_PERIOD)*(_gyroZ<<16)/(65534); // using (2^15-1)*2
  }
  // maintain angle between +-360
  if (_angle >= (360<<16)) {
    _angle -= 360<<16;
  } else if (_angle <= (-360<<16)) {
    _angle += 360<<16;
  }
  // compute frame number
  _frame = _angle/((180<<16)/NUM_FRAMES); // _angle/(1.5<<16)
  if (_frame < 0) {
    _frame = -1*_frame;
  }
  // invert frame if between beyond NUM_FRAMES
  if (_frame >= NUM_FRAMES) {
      _frame -= NUM_FRAMES;
      _reverse = true;
  } else {
      _reverse = false;
  }
}


// Convert RGB LED data (in the form of length-128 uint16_t red/green/blue arrays,
// for 128 LEDs total) to the transposed form necessary for DMAing to the eight LED
// strings on the board, connected to PTC0 through PTC7.
//
// This function also accounts for the position and orientation of each 16-LED string,
// so that the caller may imagine the LEDs as a single length-128 string from one end
// of the board to the other.  (In fact, there are only 126 LEDs because the first
// and last were dropped from the PCB, but we ignore this for the sake of symmetry.)
//
// The offset "os" allows shifting a color pattern in either direction, circularly,
// without needing to shift the data in the R/G/B arrays.  This could be useful for
// geometric effects, but not for painting an image, where every LED pattern will be
// different and not just shifted.  Set to zero if unused.
//
// EJ (6/12/24) Added the ability to invert the image. Set "inv" to true to invert from
// normal orientation.
// 
// The central operation is a "bitwise transpose" of 8x8 bit arrays, each one
// representing a single byte of LED data across the eight chains.  (See the 
// Transpose8rS32 function.)
void DLEDS::swizzle(const uint16_t R[], const uint16_t G[], const uint16_t B[], int os, bool inv)
{
    uint32_t x, y; 

    for (int i = 0 ; i < NUM_LEDS_PER_STRING; i++)  // k is the LED # within a string (0-15)
    {
        int k = inv ? (NUM_LEDS_PER_STRING-1-i) : i;
        int m = inv ? 4 : 0;
        int j = 48*(i+1);


        // Each "stanza" below packs an 8x8 bit array into two uint32_t's, which is then bit-wise
        // transposed and copied into the DMA transmit buffer, starting at the end and working
        // backwards.  Each of these 8x8 bit arrays comprises one byte of the RGB data for each of
        // the 8 LED strings.
        //
        // The WS28xx LEDs take color values in GRB (green, red, blue) order, MSB first.
        
        // Blue, low byte
        setPartLow(&x, (uint32_t) (B[(k + (7-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (B[(k + (6-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (B[(k + (5-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (B[(k + (4-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff), inv);
        setPartLow(&y, (uint32_t) (B[(NUM_LEDS_PER_STRING-1-k + (0+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (B[(NUM_LEDS_PER_STRING-1-k + (1+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (B[(NUM_LEDS_PER_STRING-1-k + (2+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (B[(NUM_LEDS_PER_STRING-1-k + (3+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff), inv);
        transpose8rS32(&x, &y);
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x;

        // Blue, high byte
        setPartHigh(&x, (uint32_t) (B[(k + (7-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (B[(k + (6-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (B[(k + (5-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (B[(k + (4-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00), inv);
        setPartHigh(&y, (uint32_t) (B[(NUM_LEDS_PER_STRING-1-k + (0+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (B[(NUM_LEDS_PER_STRING-1-k + (1+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (B[(NUM_LEDS_PER_STRING-1-k + (2+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (B[(NUM_LEDS_PER_STRING-1-k + (3+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00), inv);
        transpose8rS32(&x, &y);
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x;

        // Red, low byte
        setPartLow(&x, (uint32_t) (R[(k + (7-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (R[(k + (6-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (R[(k + (5-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (R[(k + (4-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff), inv);
        setPartLow(&y, (uint32_t) (R[(NUM_LEDS_PER_STRING-1-k + (0+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (R[(NUM_LEDS_PER_STRING-1-k + (1+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (R[(NUM_LEDS_PER_STRING-1-k + (2+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (R[(NUM_LEDS_PER_STRING-1-k + (3+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff), inv);
        transpose8rS32(&x, &y);
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x;

        // Red, high byte
        setPartHigh(&x, (uint32_t) (R[(k + (7-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (R[(k + (6-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (R[(k + (5-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (R[(k + (4-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00), inv);
        setPartHigh(&y, (uint32_t) (R[(NUM_LEDS_PER_STRING-1-k + (0+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (R[(NUM_LEDS_PER_STRING-1-k + (1+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (R[(NUM_LEDS_PER_STRING-1-k + (2+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (R[(NUM_LEDS_PER_STRING-1-k + (3+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00), inv);
        transpose8rS32(&x, &y);
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x;

        // Green, low byte
        setPartLow(&x, (uint32_t) (G[(k + (7-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (G[(k + (6-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (G[(k + (5-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (G[(k + (4-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff), inv);
        setPartLow(&y, (uint32_t) (G[(NUM_LEDS_PER_STRING-1-k + (0+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (G[(NUM_LEDS_PER_STRING-1-k + (1+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (G[(NUM_LEDS_PER_STRING-1-k + (2+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff),
                       (uint32_t) (G[(NUM_LEDS_PER_STRING-1-k + (3+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff), inv);
        transpose8rS32(&x, &y);
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x;

        // Green, high byte
        setPartHigh(&x, (uint32_t) (G[(k + (7-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (G[(k + (6-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (G[(k + (5-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (G[(k + (4-m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00), inv);
        setPartHigh(&y, (uint32_t) (G[(NUM_LEDS_PER_STRING-1-k + (0+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (G[(NUM_LEDS_PER_STRING-1-k + (1+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (G[(NUM_LEDS_PER_STRING-1-k + (2+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00),
                        (uint32_t) (G[(NUM_LEDS_PER_STRING-1-k + (3+m)*NUM_LEDS_PER_STRING + os) & (NUM_LEDS-1)] & 0xff00), inv);
        transpose8rS32(&x, &y);
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y; y >>= 8;
        _dmaBuf[--j] = y;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x; x >>= 8;
        _dmaBuf[--j] = x;
    }
}


// Set parts of a uint32 (low)
// reverse with inv = true
void DLEDS::setPartLow(uint32_t *x, uint32_t first, uint32_t second, uint32_t third, uint32_t fourth, bool inv) {
    if (!inv) {
    *x = (first << 24) | (second << 16) | (third << 8) | fourth;
    } else {
    *x = (fourth << 24) | (third << 16) | (second << 8) | first;
    }
}


// Set parts of a uint32 (high)
// reverse with inv = true
void DLEDS::setPartHigh(uint32_t *x, uint32_t first, uint32_t second, uint32_t third, uint32_t fourth, bool inv) {
    if (!inv) {
    *x = (first << 16) | (second << 8) | third | (fourth >> 8);
    } else {
    *x = (fourth << 16) | (third << 8) | second | (first >> 8);
    }
}


// An algorithm for the bit-wise transpose of an eight-element by eight-bit array, from
// "Hacker's Delight," Section 7.3: Transposing a Bit Matrix.  The eight bytes are
// pre-packed into uint32_t x and y.  This version for 32-bit architectures is found at
// https://github.com/hcs0/Hackers-Delight/blob/master/transpose8.c.txt
// (I have kept the name.)
void DLEDS::transpose8rS32(uint32_t *x, uint32_t *y)
{
    uint32_t t;

    t = (*x ^ (*x >>  7)) & 0x00aa00aa;  *x = *x ^ t ^ (t <<  7);
    t = (*y ^ (*y >>  7)) & 0x00aa00aa;  *y = *y ^ t ^ (t <<  7);

    t = (*x ^ (*x >> 14)) & 0x0000cccc;  *x = *x ^ t ^ (t << 14);
    t = (*y ^ (*y >> 14)) & 0x0000cccc;  *y = *y ^ t ^ (t << 14);

    t = (*x & 0xf0f0f0f0) | ((*y >> 4) & 0x0f0f0f0f);
    *y = ((*x << 4) & 0xf0f0f0f0) | (*y & 0x0f0f0f0f);
    *x = t;
}
