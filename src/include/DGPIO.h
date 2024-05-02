#ifndef DGPIO_H
#define DGPIO_H

class DGPIO
{
public:
    enum GPIOName
    {
        LED_RED = 0,
        LED_GREEN,
        LED_BLUE,
        ENCODER1,
        ENCODER2,
        SW2,
        SW3,
        UART0_RX,
        UART0_TX,
        LEDBAR_LOW,
        LEDBAR_HIGH,
        LEDBAR_SEG0,
        LEDBAR_SEG1,
        LEDBAR_SEG2,
        LEDBAR_SEG3,
        LEDBAR_SEG4,
        LEDBAR_SEG5,
        LEDBAR_SEG6,
        LEDBAR_SEG7,
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

    DGPIO();
    
    // Public API
    void Init();

    bool Status(GPIOName name);
    void Set(GPIOName name);
    void Clear(GPIOName name);
    void Toggle(GPIOName name);

    // Interrupt Handlers for Ports A and D
    void IRQHandler();

private:
    DGPIO(const DGPIO&);
    void operator=(const DGPIO&);

    static unsigned _interruptableGpioIndecies[NUM_GPIONAMES]; // lists the indecies in gpios that have interrupts enabled
    static unsigned _numInterruptableGpioIndecies; // number of GPIOs that can have interrupts
};

// Every user of the GPIO driver class will get this when they include DGPIO.h.
// It tells the linker that there is an instance of DGPIO called g_gpio---the one and
// only instance of the GPIO driver, typically instantiated at the top of main.cpp.
extern DGPIO g_gpio;

#endif  // DGPIO_H
