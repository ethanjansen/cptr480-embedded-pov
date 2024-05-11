#ifndef DMOTION_H
#define DMOTION_H

#include "DSPI.h"

// Static Motion Driver for LSM6DSL accelerometer/gyroscope.
// uses DSPI for communication. -- "spies" config is set up there.
// This handles Chip Select (SS/CS) via GPIO.
// Should only initialize 15ms after power on!!
// Will initialize DSPI and DGPIO (and DUART for logging).
class DMOTION {
    public:
    // Public API

    // enums
    enum {
        MAXDATA_RAW = 13, // 13 bytes of data (including address byte) (address + 6 accel + 6 gyro)
        MAXDATA = MAXDATA_RAW - 1, // 12 bytes of data (6 accel + 6 gyro)

        DMOTION_LOGGING = 1, // 0 to not log, log otherwise
    };

    // methods

    // Init.
    // Blocking while LSM6DSL is being initialized.
    // Returns false if successful.
    static bool init();

    // Check module initialization status
    static inline bool isInit() { return _init; }

    // Probe if LSM6DSL is connected -- blocking.
    // Looks for whoami register.
    // Returns True if connected.
    static bool probe();

    // Get Accelerometer Data.
    static void getAccel(signed short *x, signed short *y, signed short *z);

    // Get Gyroscope Data.
    static void getGyro(signed short *x, signed short *y, signed short *z);

    // Get Acceleromter + Gyroscope Data.
    static void getMotion(signed short *ax, signed short *ay, signed short *az, signed short *gx, signed short *gy, signed short *gz);

    // Check if LSM6DSL is busy.
    // Returns True if busy.
    static bool busy();

    // End Transaction - part of interrupt handler.
    // should only be called from DSPI to handle transaction finished.
    // This sets GPIO CC pin idle high to end transaction.
    // Special case for "getMotion()" data.
    static void endTransaction();

    // INT1/INT2 interrupt handlers.
    // These should be called from GPIO interrupt handler.
    static void INT1Handler();
    static void INT2Handler();


    private:
    // No copy, assignment, or constructor
    DMOTION();
    DMOTION(const DMOTION&);
    void operator=(const DMOTION&);

    // Private API

    // enums
    enum LSM6DSLAddr {
        // interrupts
        INT1_CTRL = 0x0D,
        INT2_CTRL,

        // who am i
        WHOAMI,

        // control
        CTRL1_XL,
        CTRL2_G,
        CTRL3_C,
        CTRL4_C,
        CTRL5_C = 0x14,
        CTRL6_C,
        CTRL7_G,

        // status
        STATUS_REG = 0x1E,

        // data
        OUTX_L_G = 0x22,
        OUTX_H_G,
        OUTY_L_G,
        OUTY_H_G,
        OUTZ_L_G,
        OUTZ_H_G,
        OUTX_L_XL,
        OUTX_H_XL,
        OUTY_L_XL,
        OUTY_H_XL,
        OUTZ_L_XL,
        OUTZ_H_XL,
    };

    enum LSM6DSLVal {
        // who am i
        WHOAMI_VAL = 0x6A,

        // control defaults
        INT1_CTRL_VAL_DEFAULT = 0,
        INT2_CTRL_VAL_DEFAULT = 0,
        CTRL1_XL_VAL_DEFAULT = 0,
        CTRL2_G_VAL_DEFAULT = 0,
        CTRL3_C_VAL_DEFAULT = 0x04, // IF_INC = 1
        CTRL4_C_VAL_DEFAULT = 0,
        CTRL5_C_VAL_DEFAULT = 0,
        CTRL6_C_VAL_DEFAULT = 0,
        CTRL7_G_VAL_DEFAULT = 0,
        TAP_CFG_VAL_DEFAULT = 0,

        // control registers with setup:

        // reset
        CTRL3_C_VAL_RESET = 0x05, // should actually wait 50us after reset

        // interrupts
        CTRL4_C_VAL_INT2_ON_INT1 = 0x20, // When using only INT1 for interrupts -- I am not sure what this does
        INT1_CTRL_VAL_XL = 1, // accel DRDY enabled
        INT1_CTRL_VAL_G = 2, // gyro DRDY enabled
        INT1_CTRL_VAL_BOTH = 3, // accel and gyro DRDY enabled
        INT2_CTRL_VAL_XL = 1, // accel DRDY enabled
        INT2_CTRL_VAL_G = 2, // gyro DRDY enabled
        INT2_CTRL_VAL_BOTH = 3, // accel and gyro DRDY enabled

        // ODR
        CTRL1_XL_VAL_12_5HZ = 0x10, // 12.5Hz (low/high)
        CTRL1_XL_VAL_26HZ = 0x20, // 26Hz (low/high)
        CTRL1_XL_VAL_52HZ = 0x30, // 52Hz (low/high)
        CTRL1_XL_VAL_104HZ = 0x40, // 104Hz (normal/high)
        CTRL1_XL_VAL_208HZ = 0x50, // 208Hz (normal/high)
        CTRL1_XL_VAL_416HZ = 0x60, // 416Hz
        CTRL1_XL_VAL_833HZ = 0x70, // 833Hz
        CTRL1_XL_VAL_1660HZ = 0x80, // 1660Hz
        CTRL1_XL_VAL_3330HZ = 0x90, // 3330Hz
        CTRL1_XL_VAL_6660HZ = 0xA0, // 6660Hz
        CTRL2_G_VAL_12_5HZ = 0x10, // 12.5Hz (low/high)
        CTRL2_G_VAL_26HZ = 0x20, // 26Hz (low/high)
        CTRL2_G_VAL_52HZ = 0x30, // 52Hz (low/high)
        CTRL2_G_VAL_104HZ = 0x40, // 104Hz (normal/high)
        CTRL2_G_VAL_208HZ = 0x50, // 208Hz (normal/high)
        CTRL2_G_VAL_416HZ = 0x60, // 416Hz
        CTRL2_G_VAL_833HZ = 0x70, // 833Hz
        CTRL2_G_VAL_1660HZ = 0x80, // 1660Hz
        CTRL2_G_VAL_3330HZ = 0x90, // 3330Hz
        CTRL2_G_VAL_6660HZ = 0xA0, // 6660Hz

        // BDU
        CTRL3_C_VAL_BDU = 0x44, // BDU and IF_INC enabled

        // rounding
        CTRL5_C_VAL_ROUND_XL = 0x20, // accel only rounding enabled
        CTRL5_C_VAL_ROUND_G = 0x40, // gyro only rounding enabled
        CTRL5_C_VAL_ROUND_BOTH = 0x60, // accel and gyro rounding enabled

        // power mode
        CTRL6_C_VAL_VAR_POWER_XL = 0x10, // accel variable power mode enabled
        CTRL7_G_VAL_VAR_POWER_G = 0x80, // gyro variable power mode enabled
    };

    // methods

    // read length registers starting at addr.
    // length <= MAXDATA.
    // Returns False if transaction start successful.
    static bool _readReg(LSM6DSLAddr addr, unsigned length);

    // write to register at addr.
    // Returns False if transaction start successful.
    static bool _writeReg(LSM6DSLAddr addr, char data);

    // blocking _writeReg for setup.
    static bool _writeRegBlocking(LSM6DSLAddr addr, char data);

    // blocking _readReg.
    // for testing (and for probe).
    static bool _readRegBlocking(LSM6DSLAddr addr, unsigned length);


    // Private Data
    static bool _init;

    // An extra flag to protect from reading twice during init.
    // This simply protects from improper init sequence (if interrupts are enabled last this shouldn't be necessary).
    // I tested this by trying to config stuff after enabling interrupts on INT1 and it properly blocked an early interrupt!
    static bool _receivedBlockedDataDuringInit;

    // flag for accel/gyro transaction
    static bool _inMotionTransaction;

    // accel/gyro data
    static signed short _accel[3], _gyro[3]; // stored in order X, Y, Z

    // Keep in/out data buffers here to prevent undefined behavior.
    // if RAM is tight could probably make _outData have size 1 (though this would require some refactoring).
    static char _inData[MAXDATA_RAW], _outData[MAXDATA_RAW];
};


#endif // DMOTION_H