#include "DSPI.h"
#include "MKL25Z4.h"
// for callbacks
#include "DMOTION.h"

// Static var initialization

// configuration used at init()
const DSPI::SPIConfig DSPI::spies[] = { // haha "spies" :)
 // spiName, spiMode, clkPolarity, clkPhase, bitOrder, baud, transactionFinishedCallback
    {SPI_1, MASTER, IDLE_HIGH, TX_1ST_EDGE, MSB_FIRST, BAUD_1MHZ, DMOTION::endTransaction},
};

// busy flags
bool DSPI::_spiInUse[NUM_SPINames]; // {false, false}

// TX/RX buffers
char *DSPI::_inData[NUM_SPINames]; // {nullptr, nullptr}
char *DSPI::_outData[NUM_SPINames]; // {nullptr, nullptr}
unsigned DSPI::_lengthTX[NUM_SPINames]; // {0, 0}
unsigned DSPI::_lengthRX[NUM_SPINames]; // {0, 0}

bool DSPI::_init;


// public methods

// Initialize SPI
void DSPI::init() {
    for (unsigned i=0; i<sizeof(spies)/sizeof(SPIConfig); i++) {
        // enable clock
        SIM->SCGC4 |= SIM_SCGC4_SPI0_MASK << spies[i].spiName;

        // set up SPI module
        SPI_Type *spi = (SPI_Type *)(SPI0_BASE + (0x1000 * spies[i].spiName));

        // mode, polarity, phase, bitOrder
        spi->C1 |= SPI_C1_MSTR(spies[i].spiMode) | SPI_C1_CPOL(spies[i].clkPolarity) | SPI_C1_CPHA(spies[i].clkPhase) | SPI_C1_LSBFE(spies[i].bitOrder);

        // baud rate (reduce SPR for SPI_0)
        unsigned baud = spies[i].baud & 0x7F;
        if (spies[i].spiName == SPI_0) {
            baud--;
        }
        spi->BR = baud;

        // enable SPI
        spi->C1 |= SPI_C1_SPE_MASK;

        // interrupt enable
        NVIC_EnableIRQ((IRQn_Type)((unsigned)SPI0_IRQn+spies[i].spiName));
    }

    _init = true;
}

// ### Transmit/Receive Simulaneously:
//  - inData and _outData must be same length (length bytes).
//  - typically, first byte of inData is nonsense (dummy byte) received during first byte sent of _outData.
//  - Ex: If you want to read 1 byte and send 1 byte, inData and _outData should be 2 bytes long (length==2).
// #### This is non-blocking, ensure that pointers stay valid during entire transaction.
// #### This does not handle Slave Select (SS) pin -- should be handled externally via GPIO.
// #### Returns True and does nothing if transaction is already in progress.
bool DSPI::TxRx(SPIName spi, unsigned length, char *inData, char *outData) {
    // check if spi is busy -- or invalid inputs
    if (_spiInUse[spi] | !inData | !outData | !length) {
        return true;
    }

    // set up class vars -- in, out, length, flag
    _inData[spi] = inData;
    _outData[spi] = outData;
    _lengthRX[spi] = length;
    _lengthTX[spi] = length;
    _spiInUse[spi] = true;

    // enable interrupts
    SPI_Type *spiAddr = (SPI_Type *)(SPI0_BASE + (0x1000 * spi));
    spiAddr->C1 |= SPI_C1_SPIE_MASK | SPI_C1_SPTIE_MASK; // let interrupt handler send data

    return false;
}

// Poll Status.
// No internal buffer, only read/write once at a time.
bool DSPI::busy(SPIName spi) {
    return _spiInUse[spi];
}

// Interrupt Handler
void DSPI::IRQHandler() {
    for (unsigned i = 0; i < sizeof(spies)/sizeof(SPIConfig); i++) {
        SPIName j = spies[i].spiName; // spi name
        SPI_Type *spi = (SPI_Type *)(SPI0_BASE + (0x1000 * j));
        if (spi->S & SPI_S_SPTEF_MASK) {
            // transmit
            if (_lengthTX[j]) {
                spi->D = *_outData[j]; // flag is automatically cleared
                _outData[j]++;
                _lengthTX[j]--;
            } else { // nothing to send
                spi->C1 &= ~SPI_C1_SPTIE_MASK; // disable transmit interrupt
            }
        }
        if (spi->S & SPI_S_SPRF_MASK) {
            // receive
            if (_lengthRX[j]) {
                *_inData[j] = spi->D; // flag is automatically cleared
                _inData[j]++;
                _lengthRX[j]--;
            } else { // nothing to receive
                spi->C1 &= ~SPI_C1_SPIE_MASK; // disable receive interrupt
            }
        }

        // check if transaction is complete
        if (!_lengthTX[j] && !_lengthRX[j]) {
            _spiInUse[j] = false;
            _inData[j] = nullptr;
            _outData[j] = nullptr;

            if (spies[j].transactionFinishedCallback) {
                spies[j].transactionFinishedCallback(); // note transaction is finished
            }
        }

        // this shouldn't happen, but just in case
        if (spi->S & SPI_S_MODF_MASK) {
            // clear MODF
            spi->C1 &= ~SPI_C1_SSOE_MASK; // cleared by writing to C1
        }
    }
}
