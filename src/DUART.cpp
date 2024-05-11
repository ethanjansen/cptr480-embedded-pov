#include "DUART.h"
#include "MKL25Z4.h"
#include "int2str.h"

// constant configuration
const DUART::UARTConfig DUART::uartConfigs[] = {
//  {name,   baudRate,    parity,      stopBits,    bitMode,    inversion, direction}
    {UART_0, BAUD_115200, PARITY_NONE, STOP_BITS_1, DATA_BITS_8, NORMAL, LSB_FIRST},
};

// static var initialization
DUART::Buffer DUART::_buffer;
bool DUART::_init;

// Public UART Initialization API
// Enable clocks: UART0 (configure clock source), PORTA
// Set baud rate, stop bits, parity bits (mode), bit mode, [inversion, and direction (MSB vs LSB)].
// Enable UART pin mux (UART0 on pins PTA1 and PTA2 (ALT2))
// Enable transmitter (and receiver)
unsigned DUART::init() {
    for (unsigned i = 0; i < sizeof(uartConfigs)/sizeof(uartConfigs[0]); i++) {
        switch (uartConfigs[i].name) {
            case UART_0:
                // Enable UART0 clock
                SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
                // Set UART0 clock source to MCGPLLCLK/2
                SIM->SOPT2 |= SIM_SOPT2_UART0SRC(1) | SIM_SOPT2_PLLFLLSEL_MASK;
                // Set UART0 baud rate to uartConfigs[i].baudRate
                // OSR is 16x
                // also set number of stop bits
                UART0->BDL = uartConfigs[i].baudRate & 0xFF;
                UART0->BDH |= ((uartConfigs[i].baudRate >> 8) & 0x1F) | (uartConfigs[i].stopBits & 0x20);
                // Set UART0 parity, bit mode, inversion, and direction
                UART0->C1 |= (uartConfigs[i].parity & 0x3) | (uartConfigs[i].bitMode & 0x10);
                UART0->C4 |= uartConfigs[i].bitMode & 0x20;
                UART0->C3 |= uartConfigs[i].inversion & 0x10;
                UART0->S2 |= (uartConfigs[i].inversion & 0x10) | (uartConfigs[i].direction & 0x20);
                // Enable interrupts in NVIC
                NVIC_EnableIRQ(UART0_IRQn);
                // Enable UART0 transmitter
                UART0->C2 |= UART0_C2_TE_MASK;
                // Receiver not implemented yet...
                break;
            default:
                return 1; // no other uart modules implemented yet...
        }
    }

    // clear buffer on init
    clearBuffer();
    _init = true;
    return 0;
}

// send a string by filling the internal buffer and enabling interrupts
// returns 0 on success, 1 if buffer does not have enough space 
unsigned DUART::sendString(const char *str) {
    // check if string is empty, immediately return
    if (str[0] == '\0') {
        return 0;
    }

    // check if buffer is full and immediately return if necessary
    if (!getBufferFreeSpace()) {
        return 1;
    }

    // fill buffer with string until null terminator or tempTail reaches headIndex
    // do one iteration outside of loop if empty -- I am trying to optimize, probably a bad idea
    unsigned tempTail = _buffer.tailIndex;
    unsigned i = 0;
    if (_buffer.isEmpty) {
        // add str character to buffer
        _buffer.data[tempTail] = str[0];
        // increment with wrapping
        tempTail = (tempTail + 1) & (Buffer::BUFFER_SIZE - 1); // fast module for wrap around
        i++;
    }
    
    while (str[i] != '\0') {
        // check if buffer is full while not at the end of str, return immediately
        if (tempTail == _buffer.headIndex) {
            return 1;
        }
        // add str character to buffer
        _buffer.data[tempTail] = str[i];
        // increment with wrapping
        tempTail = (tempTail + 1) & (Buffer::BUFFER_SIZE - 1); // fast module for wrap around
        i++;
    }

    // we did not try to overfill buffer and successfully added str.
    // update _buffer tailIndex and isEmpty flag
    _buffer.tailIndex = tempTail;
    _buffer.isEmpty = 0;

    // enable UART0 interrupts: TDRE
    UART0->C2 |= UART_C2_TIE_MASK;

    // return success
    return 0;
}

// send an int as an ASCII string, using base (2-16). End with terminator.
// Will always use '-' for negative rather than 2's complement.
// returns 0 on success, 1 if buffer does not have enough space
unsigned DUART::sendInt(signed num, signed base, char terminator) {
    unsigned maxLength;

    // dynamically find max length for common bases (don't forget sign)
    switch (base) {
        case 8: // octal
            maxLength = 13; // 11 data bits, 1 sign, 1 null terminator
            break;
        case 10: // decimal
            maxLength = 12; // 10 data bits, 1 sign, 1 null terminator
            break;
        case 16: // hex
            maxLength = 9; // 8 data bits, 1 sign, 1 null terminator
            break;
        case 2: // binary
        default: // use largest
            maxLength = 33; // 31 data bits, 1 sign, 1 null terminator
            break;
    };

    char str[maxLength+1]; // extra for "terminator"
    int2str(num, base, str, maxLength); // 12 is max length of int in base 2???
    // add "terminator" and swap with null terminator
    str[maxLength-1] = terminator;
    str[maxLength] = '\0';

    // send string
    return sendString(str);
}

// simply disabled UART modules in uartConfigs
// does not disable interrupts!
void DUART::disable() {
    for (unsigned i = 0; i < sizeof(uartConfigs)/sizeof(uartConfigs[0]); i++) {
        switch (uartConfigs[i].name) {
            case UART_0:
                // Disable UART0 transmitter (and receiver)
                // UART0->C2 &= ~(UART0_C2_TE_MASK | UART0_C2_RE_MASK); // receiver not implemted yet...
                UART0->C2 &= ~UART0_C2_TE_MASK;
                break;
            default:
                // no other uart modules implemented yet...
                break;
        }
    }

    _init = false; // Need to reinitialize to use again
}

// get the number of bytes free in the buffer
unsigned DUART::getBufferFreeSpace() {
    if (_buffer.isEmpty) {
        return Buffer::BUFFER_SIZE;
    } else if (_buffer.headIndex >= _buffer.tailIndex) {
        return _buffer.headIndex - _buffer.tailIndex;
    } else {
        return Buffer::BUFFER_SIZE - _buffer.tailIndex + _buffer.headIndex;
    }
}

// clear buffer
void DUART::clearBuffer() {
    _buffer.isEmpty = 1;
    _buffer.headIndex = 0;
    _buffer.tailIndex = 0;
}

// Interrupt handler
// Currently only handles TDRE for UART0
// TDRE in S1 (bit 7) is set when the buffer is empty
// CHAT, need to check
void DUART::IRQHandler() {
    // only handle TDRE
    if (!(UART0->S1 & UART_S1_TDRE_MASK)) {
        return;
    }

    // this should really never happen, but just in case
    if (_buffer.isEmpty) {
        // disable interrupts
        UART0->C2 &= ~UART_C2_TIE_MASK;
        return;
    }

    // UART buffer is empty, send more data
    UART0->D = _buffer.data[_buffer.headIndex]; // 1 byte at a time
    _buffer.headIndex = (_buffer.headIndex + 1) & (Buffer::BUFFER_SIZE - 1); // fast module for wrap around

    // check if buffer is empty, use headIndex==tailIndex instead of isEmpty.
    // if isEmpty is set we should not even be here!
    // if so, set isEmpty and disable interrupts
    if (_buffer.headIndex == _buffer.tailIndex) {
        _buffer.isEmpty = 1;
        UART0->C2 &= ~UART_C2_TIE_MASK;
    }    

    // clear all writable interrupt flags
    UART0->S1 |= 0x1F; // 0b11111
    UART0->S2 |= 0xC0; // 0b11000000
}