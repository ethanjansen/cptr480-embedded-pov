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
        INT_LogicZero,
        INT_RisingEdge,
        INT_FallingEdge,
        INT_BothEdges,
        INT_LogicOne,
        NUM_GPIOINTERRUPTS
    };
        
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
    };

    struct GPIOTableSmall // holds less information than GPIOTable
    {
        enum GPIOName name : 8; // name
        unsigned pin       : 8; // pin number
        enum GPIOPort port : 4; // port name
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

    static GPIOTableSmall _interruptableGpios[NUM_GPIONAMES]; // lists what GPIOs can have interrupts
    static unsigned _numInterruptableGpios; // number of GPIOs that can have interrupts
};

// Every user of the GPIO driver class will get this when they include DGPIO.h.
// It tells the linker that there is an instance of DGPIO called g_gpio---the one and
// only instance of the GPIO driver, typically instantiated at the top of main.cpp.
extern DGPIO g_gpio;

#endif  // DGPIO_H
