#ifndef DSPI_H
#define DSPI_H

// Static Full-Duplex (Master Mode) SPI Driver Class.
// Interrupt Based Implementation, no DMA (yet).
// Pin muxing should be handled by DGPIO.
class DSPI {
    public:
    // Public API

    // types
    enum SPIName {
        SPI_0, // on bus clock (24MHz)
        SPI_1, // on system clock (48MHz)
        NUM_SPINames,
    };

    enum SPIMode {
        //SLAVE, // This is not implemented
        MASTER = 1,
    };

    enum CLKPolarity {
        IDLE_LOW,
        IDLE_HIGH,
    };

    enum CLKPhase {
        TX_2ND_EDGE,
        TX_1ST_EDGE,
    };

    enum SPIBitOrder {
        MSB_FIRST,
        LSB_FIRST,
    };

    // Default Supported Baud Rates.
    // Maybe add more in the future...
    // Format: SPPR[2:0],SPR[3:0] (in binary).
    // Assume 48MHz system clock input (if using SPI_0, the SPR[3:0] value will be reduced automatically).
    // 4 MHz seems to be the limit. Haun did not have much success at 8MHz and at 6MHz I am not receiving all bits...
    enum SPIBaudRate {
        BAUD_75KHZ =  0b1000110, // SPPR = 0b100, SPR = 0b0110
        BAUD_150KHZ = 0b1000101, // SPPR = 0b100, SPR = 0b0101
        BAUD_250KHZ = 0b0100101, // SPPR = 0b010, SPR = 0b0101
        BAUD_500KHZ = 0b0100100, // SPPR = 0b010, SPR = 0b0100
        BAUD_1MHZ =   0b0100011, // SPPR = 0b010, SPR = 0b0011
        BAUD_2MHZ =   0b0100010, // SPPR = 0b010, SPR = 0b0010
        BAUD_3MHZ =   0b0110001, // SPPR = 0b011, SPR = 0b0001
        BAUD_4MHZ =   0b0100001, // SPPR = 0b010, SPR = 0b0001
        //BAUD_6MHZ =   0b0010001, // SPPR = 0b001, SPR = 0b0001
        //BAUD_12MHZ =  0b0000001, // SPPR = 0b000, SPR = 0b0001
    };

    // Optional interrupt handler callback.
    // This will be called when the transaction is finished.
    // Flags will be cleared before this is called.
    // This should NOT handle SPI stuff, just application logic.
    typedef void (*extCallback)(void);

    struct SPIConfig {
        SPIName spiName;
        SPIMode spiMode;
        CLKPolarity clkPolarity;
        CLKPhase clkPhase;
        SPIBitOrder bitOrder;
        SPIBaudRate baud;
        extCallback transactionFinishedCallback;
    };

    // class vars

    // configuration used at init()
    static const SPIConfig spies[]; // haha "spies" :)

    // methods

    // Initialize SPI
    static void init();

    // Check module initialization status
    static inline bool isInit() { return _init; }

    // Transmit/Receive Simulaneously:
    //  - inData and _outData must be same length (length bytes).
    //  - typically, first byte of inData is nonsense (dummy byte) received during first byte sent of _outData.
    //  - Ex: If you want to read 1 byte and send 1 byte, inData and _outData should be 2 bytes long (length==2).
    // This is non-blocking, ensure that pointers stay valid during entire transaction.
    // This does not handle Slave Select (SS) pin -- should be handled externally via GPIO.
    // Returns True and does nothing if transaction is already in progress.
    static bool TxRx(SPIName spi, unsigned length, char *inData, char *outData);

    // Poll Status.
    // No internal buffer, only read/write once at a time.
    static bool busy(SPIName spi);

    // Interrupt Handler
    static void IRQHandler();


    private:
    // No copy, assignment, or constructor
    DSPI();
    DSPI(const DSPI&);
    void operator=(const DSPI&);

    // Private data
    static bool _init;

    static bool _spiInUse[NUM_SPINames];

    static char *_inData[NUM_SPINames], *_outData[NUM_SPINames];
    static unsigned _lengthRX[NUM_SPINames], _lengthTX[NUM_SPINames];
};


#endif // DSPI_H