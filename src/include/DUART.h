#ifndef DUART_H
#define DUART_H

// Interrupt-based Driver for UART0 module
// Only enables transmitter
class DUART {
    public:
        // enums:

        enum UARTNames {
            UART_0,
            //UART_1, // not implemented
            //UART_2, // not implemented
            NUM_UARTNames,
        };

        // special divisor values
        // using 48MHz clock and 16x OSR
        enum UARTBaudRate {
            BAUD_300 = 10000, // 1000
            BAUD_1200 = 2500, // 2500
            BAUD_2400 = 1250, // 1250
            BAUD_4800 = 625, // 625
            BAUD_9600 = 312, // 312.5 (312)
            BAUD_14400 = 208, // 208.333333333333 (208)
            BAUD_19200 = 156, // 156.25 (156)
            BAUD_38400 = 78, // 78.125 (78)
            BAUD_57600 = 52, // 52.083333333333333 (52)
            BAUD_115200 = 26, // 26.04166666666667 (26)
            NUM_UARTBAUDRATES = 10,
        };

        enum UARTParity {
            PARITY_NONE, // default
            PARITY_EVEN = 2,
            PARITY_ODD,
        };

        enum UARTStopBits {
            STOP_BITS_1, // default
            STOP_BITS_2 = 0x20,
        };

        // number of data bits
        enum UARTBitMode {
            DATA_BITS_8, // default
            //DATA_BITS_9 = 0x10, // not implemented
            //DATA_BITS_10 = 0x20, // not implemented
        };

        enum UARTInversion {
            NORMAL, // default
            INVERTED = 0x10,
        };

        enum UARTDirection {
            LSB_FIRST, // default
            MSB_FIRST = 0x20,
        };

        // configuration struct
        struct UARTConfig {
            UARTNames name;
            UARTBaudRate baudRate;
            UARTParity parity;
            UARTStopBits stopBits;
            UARTBitMode bitMode;
            UARTInversion inversion;
            UARTDirection direction;
        };

        // UART module configuration
        // set before calling init()
        // constant for compile-time configuration
        static const UARTConfig uartConfigs[];

        // initialize UART module.
        // requires configuration and UART number (currently limited to UART0).
        // Requires GPIO configuration for UART for communication over OpenSDA.
        static unsigned init();

        // Check initialization status
        static inline bool isInit() { return _init; }

        // disable UART module
        static void disable();

        // send a string by filling the internal buffer and enabling interrupts
        // returns 0 on success, 1 if buffer does not have enough space 
        static unsigned sendString(const char *str);

        // send an int as an ASCII string, using base (2-16).
        // returns 0 on success, 1 if buffer does not have enough space
        static unsigned sendInt(signed num, signed base);

        // get the number of bytes free in the buffer
        static unsigned getBufferFreeSpace();

        // clear buffer
        static void clearBuffer();

        // Interrupt handler
        static void IRQHandler();

    private:
        // no copy or assignment
        DUART();
        DUART(const DUART&);
        void operator=(const DUART&);

        // circular buffer struct
        struct Buffer {
            static const unsigned BUFFER_SIZE = 256; // Buffer size of 256 bytes, use power of 2
            char data[BUFFER_SIZE];
            unsigned isEmpty; // 1 if buffer is empty, 0 if not -- used for clearing
            unsigned headIndex; // next byte to read
            unsigned tailIndex; // next byte to write
        };

        // private buffer
        static Buffer _buffer;
        
        // initialization flag
        static bool _init;
};

#endif // DUART_H