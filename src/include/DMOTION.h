#ifndef DMOTION_H
#define DMOTION_H

#include "DSPI.h"

// Static Motion Driver for LSM6DSL accelerometer/gyroscope.
// uses DSPI for communication. -- "spies" config is set up there.
// This handles Chip Select (SS/CS) via GPIO.
class DMOTION {
    public:
    // Public API

    // enums
    enum {
        MAXDATA_RAW = 13, // 13 bytes of data (including address byte) (address + 6 accel + 6 gyro)
        MAXDATA = MAXDATA_RAW - 1, // 12 bytes of data (6 accel + 6 gyro)
    };

    enum LSM6DSLAddr {
        WHOAMI = 0x0F,
    };

    // methods

    // Initialize the LSM6DSL
    static void init();

    // Check module initialization status
    static inline bool isInit() { return _init; }

    // Probe if LSM6DSL is connected -- blocking.
    // Looks for whoami register.
    // Returns True if connected.
    static bool probe();

    // Check if LSM6DSL is busy.
    // Returns True if busy.
    static bool busy();

    // End Transaction - part of interrupt handler.
    // should only be called from DSPI to handle transaction finished.
    // This sets GPIO CC pin idle high to end transaction.
    static void endTransaction();


    private:
    // No copy, assignment, or constructor
    DMOTION();
    DMOTION(const DMOTION&);
    void operator=(const DMOTION&);

    // Private API

    // enums
    enum LSM6DSLVal {
        WHOAMI_VAL = 0x6A,
    };

    // methods

    // read length registers starting at addr.
    // length <= MAXDATA.
    // Returns False if transaction start successful.
    static bool _readReg(LSM6DSLAddr addr, unsigned length);

    // write to register at addr.
    // Returns False if transaction start successful.
    static bool _writeReg(LSM6DSLAddr addr, char data);

    // Private Data
    static bool _init;

    // Keep in/out data buffers here to prevent undefined behavior.
    // if RAM is tight could probably make _outData have size 1 (though this would require some refactoring).
    static char _inData[MAXDATA_RAW], _outData[MAXDATA_RAW];
};


#endif // DMOTION_H