#include "DMOTION.h"
#include "DSPI.h"
#include "DGPIO.h"
#include "DUART.h" // testing

// static var initialization
bool DMOTION::_init;
bool DMOTION::_receivedBlockedDataDuringInit;
bool DMOTION::_inMotionTransaction;
signed short DMOTION::_accel[3], DMOTION::_gyro[3];
char DMOTION::_inData[MAXDATA_RAW], DMOTION::_outData[MAXDATA_RAW];

// Public API

// Init.
// Blocking while LSM6DSL is being initialized.
// Returns False if successful.
bool DMOTION::init() {
    // initialize UART for testing
    if (DMOTION_LOGGING && !DUART::isInit()) {
        DUART::init();
    }

    // initalize SPI then GPIO
    if (!DSPI::isInit()) { // using SPI_1 for communication -- ensure config is set in DSPI.h
        DSPI::init();
    }
    if (!DGPIO::isInit()) { // used for CS/SS -- ensure config is set in DGPIO.h
        DGPIO::init();
    }

    // LSM6DSL specific initalization...
    // - odr = 833Hz
    // - precision = 500dps
    // - rounding (leave default)
    // - block data update (BDU) = enabled -- only ensures low byte matches high byte
    // - power mode (leave default)
    // - interrupts = both enabled on INT1 (Not for Project!!!!)
    bool status;

    // probe (ensure LSM6DSL is connected)
    if (!probe()) {
        return true;
    }

    // clean start
    status = _writeRegBlocking(CTRL1_XL, CTRL1_XL_VAL_DEFAULT); // start accel in power down and gryo in high performance (I don't think this is actually necessary)
    status |= _writeRegBlocking(CTRL2_G, CTRL2_G_VAL_416HZ);
    status |= _writeRegBlocking(CTRL3_C, CTRL3_C_VAL_RESET); // actual reset

    // probe again, this should also delay things after the reset (ideally for 50us)
    // At 4MHz Baud, this waits for 40us, which as everyone knows is basically 50us (it seems to be sufficient).
    if (!probe()) {
        return true;
    }

    // setup
    //status |= _writeRegBlocking(CTRL1_XL, CTRL1_XL_VAL_833HZ); // ODR -- do this first to turn on device
    status |= _writeRegBlocking(CTRL2_G, CTRL2_G_VAL_833HZ | CTRL2_G_VAL_500DPS); // Project: not synced so make fast and no slower than 250HZ. However, data consistency is worse (despite BDU) passed 1660Hz.
    status |= _writeRegBlocking(CTRL3_C, CTRL3_C_VAL_BDU); // BDU
    status |= _writeRegBlocking(INT2_CTRL, INT2_CTRL_VAL_DEFAULT); // no interrupts on INT2 (prevents hanging)
    status |= _writeRegBlocking(INT1_CTRL, INT1_CTRL_VAL_DEFAULT); // no interrupts on INT2 (prevents hanging)

    _init = !status;

    return status;
}

// Probe if LSM6DSL is connected -- blocking.
// Looks for whoami register
bool DMOTION::probe() {
    // Read WHOAMI register, checking if SPI sent data
    if (_readRegBlocking(WHOAMI, 1)) {
        return false;
    }

    // Check if WHOAMI register matches expected value
    return _inData[1] == WHOAMI_VAL; // index 0 is dummy
}

// Check if LSM6DSL is busy
bool DMOTION::busy() {
    // Check SPI_1 status
    return DSPI::busy(DSPI::SPI_1);
}

// Get Accelerometer Data.
void DMOTION::getAccel(signed short *x, signed short *y, signed short *z) {
    *x = _accel[0];
    *y = _accel[1];
    *z = _accel[2];
}

// Get Gyroscope Data.
void DMOTION::getGyro(signed short *x, signed short *y, signed short *z) {
    *x = _gyro[0];
    *y = _gyro[1];
    *z = _gyro[2];
}

// Get Gyroscope Z -- local copy (only need this for Project)
// Includes correction factor
void DMOTION::getGyroZ(signed short *z) {
    *z = _gyro[2]+CORRECTION_FACTOR;
}

// Get Acceleromter + Gyroscope Data.
void DMOTION::getMotion(signed short *ax, signed short *ay, signed short *az, signed short *gx, signed short *gy, signed short *gz) {
    getAccel(ax, ay, az);
    getGyro(gx, gy, gz);
}

// SPI Call (for Project)
void DMOTION::getGyroZFromSPI(bool block) {
    // Don't perform transaction during init
    if (!_init) {
        _receivedBlockedDataDuringInit = true;
        return;
    }

    _inMotionTransaction = !_readReg(OUTZ_L_G, 2); // 2 bytes of gyro

    // should block
    while (block && _inMotionTransaction) {}
}


// Handler

// End Transaction - part of interrupt handler.
// should only be called from DSPI to handle transaction finished.
// This sets GPIO CC pin idle high to end transaction.
// Special case for "getMotion()" data.
// MODIFIED FOR PROJECT -- only get gyroZ
void DMOTION::endTransaction() {
    DGPIO::Set(DGPIO::MOTION_CC);

    if (_inMotionTransaction) {
        // parse data -- should I check some length for proper data access?
        _gyro[2] = (_inData[2] << 8) | _inData[1];

        // testing: UART logging (this is quite a bit for an interrupt handler, so only use when logging)
        if (DMOTION_LOGGING) {
            DUART::sendInt(_gyro[2], 10, '\n');
        }

        // toggle flag
        _inMotionTransaction = false;
    }
}

// Not used for project
// INT1/INT2 interrupt handlers.
// These should be called from GPIO interrupt handler.
void DMOTION::INT1Handler() { // assuming both gyro and accel interrupts on INT1, but read both
    // Don't perform transaction during init
    if (!_init) {
        _receivedBlockedDataDuringInit = true;
        return;
    }

    // read data and set flag
    _inMotionTransaction = !_readReg(OUTX_L_G, 12); // 6 bytes of gyro, 6 bytes of accel
}

void DMOTION::INT2Handler() { // DO NOT USE! -- currently just hangs
    while (true) {/* hang */}

    // Don't perform transaction during init
    if (!_init) {
        _receivedBlockedDataDuringInit = true;
        return;
    }
    
    // read data and set flag
    _inMotionTransaction = !_readReg(OUTX_L_G, 12); // 6 bytes of gyro, 6 bytes of accel
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

// blocking _writeReg for setup.
bool DMOTION::_writeRegBlocking(LSM6DSLAddr addr, char data) {
    // write reg, checking if SPI sent data
    if (_writeReg(addr, data)) {
        return true;
    }

    // wait for write to complete
    while (busy()) {}

    return false;
}

// blocking _readReg for testing.
bool DMOTION::_readRegBlocking(LSM6DSLAddr addr, unsigned length) {
    // read reg, checking if SPI sent data
    if (_readReg(addr, length)) {
        return true;
    }

    // wait for read to complete
    while (busy()) {}

    return false;
}