#ifndef DGPIO_H
#define DGPIO_H

class DGPIO
{
public:
    enum GPIOName
    {
        // LED_RED = 0,
        // LED_GREEN,
        // LED_BLUE,
        // ENCODER1,
        // ENCODER2,
        // SW2,
        // SW3,
        // UART0_RX,
        // UART0_TX,
        // SPI1_MISO,
        // SPI1_MOSI,
        // SPI1_SCK,
        // MOTION_CC,
        // MOTION_INT1,
        // MOTION_INT2,
        WAVE0,
        WAVE1,
        WAVE2,
        WAVE3,
        WAVE4,
        NUM_GPIONAMES,
    };

    enum GPIOPort
    {
        PortA = 0,
        PortB,
        PortC,
        PortD,
        PortE,
        NUM_GPIOPORTS
    };

    enum GPIOIO
    {
        Input,
        Output,
        NUM_GPIOIOS
    };

    enum GPIOPUPD
    {
        PU,
        PD,
        Float,
        NUM_GPIOPUPDS
    };

    enum GPIOInterrupt // Only Ports A and D support interrupts
    {
        Disabled,
        DMA_RisingEdge, // DMA not implemented - *Do not use*
        DMA_FallingEdge,
        DMA_BothEdges,
        INT_LogicZero = 8,
        INT_RisingEdge,
        INT_FallingEdge,
        INT_BothEdges,
        INT_LogicOne,
    };

    // use callbacks for interrupt handling -- what's the worst that can happen?
    typedef void (*interruptHandler)(void);
        
    struct GPIOTable
    {
        enum GPIOName name           : 8; // name
        unsigned pin                 : 8; // pin number
        enum GPIOPort port           : 4; // port name
        enum GPIOIO io               : 4; // input or output
        enum GPIOPUPD pupd           : 4; // pull-up or pull-down
        enum GPIOInterrupt interrupt : 4; // interrupt type     
        unsigned mux                 : 3; // mux value (0-7)
        unsigned init                : 4; // init high or low (GPIO)
        interruptHandler handler;         // Interrupt Handler
    };

    static const GPIOTable gpios[];
    
    // Public API
    static void init();
    static inline bool isInit() { return _init; }

    static bool Status(GPIOName name);
    static void Set(GPIOName name);
    static void Clear(GPIOName name);
    static void Toggle(GPIOName name);

    static void *getOutputRegister(GPIOName name);
    static void *getInputRegister(GPIOName name);

    // Interrupt Handlers for Ports A and D
    static void IRQHandler();

private:
    DGPIO();
    DGPIO(const DGPIO&);
    void operator=(const DGPIO&);

    static bool _init;

    static unsigned _interruptableGpioIndecies[NUM_GPIONAMES]; // lists the indecies in gpios that have interrupts enabled
    static unsigned _numInterruptableGpioIndecies; // number of GPIOs that can have interrupts
};

#endif  // DGPIO_H
