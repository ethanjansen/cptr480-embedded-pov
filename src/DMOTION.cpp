#include "DMOTION.h"
#include "DSPI.h"
#include "DGPIO.h"

// static var initialization
bool DMOTION::_init;
char DMOTION::_inData[MAXDATA_RAW], DMOTION::_outData[MAXDATA_RAW];

// Public API

// Initialize the LSM6DSL
void DMOTION::init() {
    // initalize SPI then GPIO
    if (!DSPI::isInit()) { // using SPI_1 for communication -- ensure config is set in DSPI.h
        DSPI::init();
    }
    if (!DGPIO::isInit()) { // used for CS/SS -- ensure config is set in DGPIO.h
        DGPIO::init();
    }

    // LSM6DSL specific initalization...
    // - rounding
    // - interrupts
    // - odr = 26Hz for now
    // - block data update (BDU) -- only ensures low byte matches high byte

    _init = true;
}

// Probe if LSM6DSL is connected -- blocking.
// Looks for whoami register
bool DMOTION::probe() {
    // Read WHOAMI register, checking if SPI sent data
    if (_readReg(WHOAMI, 1)) {
        return false;
    }

    // wait for probe to complete
    while (busy()) {} // is there a better way to do this?

    // Check if WHOAMI register matches expected value
    return _inData[1] == WHOAMI_VAL; // index 0 is dummy
}

// Check if LSM6DSL is busy
bool DMOTION::busy() {
    // Check SPI_1 status
    return DSPI::busy(DSPI::SPI_1);
}


// Handler

// End Transaction - part of interrupt handler.
// should only be called from DSPI to handle transaction finished.
// This sets GPIO CC pin idle high to end transaction.
void DMOTION::endTransaction() {
    DGPIO::Set(DGPIO::MOTION_CC);
}


// Private API

// read length registers starting at addr.
// length <= MAXDATA.
// Returns False if transaction start successful.
bool DMOTION::_readReg(LSM6DSLAddr addr, unsigned length) {
    // set up data
    _outData[0] = addr | 0x80; // read bit = 1

    // Set CC active low
    DGPIO::Clear(DGPIO::MOTION_CC);

    // Start transaction
    bool status = DSPI::TxRx(DSPI::SPI_1, length+1, _inData, _outData);

    if (status) {
        // transaction did not work, reset CC
        DGPIO::Set(DGPIO::MOTION_CC);
    }

    return status;
}

// write to register at addr.
// Returns False if transaction start successful.
bool DMOTION::_writeReg(LSM6DSLAddr addr, char data) {
    // set up data
    _outData[0] = addr & 0x7F; // write bit = 0
    _outData[1] = data;

    // Set CC active low
    DGPIO::Clear(DGPIO::MOTION_CC);

    // Start transaction
    bool status = DSPI::TxRx(DSPI::SPI_1, 2, _inData, _outData);

    if (status) {
        // transaction did not work, reset CC
        DGPIO::Set(DGPIO::MOTION_CC);
    }

    return status;
}