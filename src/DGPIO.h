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
        TPM2_CH0_EXT,
        TPM2_CH1_EXT,
        TPM0_CH1_EXT,
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
        
    struct GPIOTable
    {
        enum GPIOName name : 8;
        unsigned pin       : 8;
        enum GPIOPort port : 4;
        enum GPIOIO io     : 4;
        enum GPIOPUPD pupd : 4;
        unsigned mux       : 3; 
        unsigned init      : 4;
    };

    static const GPIOTable gpios[];

    DGPIO();
    
    // Public API
    void Init();

    bool Status(GPIOName name);
    void Set(GPIOName name);
    void Clear(GPIOName name);
    void Toggle(GPIOName name);

private:
    DGPIO(const DGPIO&);
    void operator=(const DGPIO&);
};

// Every user of the GPIO driver class will get this when they include DGPIO.h.
// It tells the linker that there is an instance of DGPIO called g_gpio---the one and
// only instance of the GPIO driver, typically instantiated at the top of main.cpp.
extern DGPIO g_gpio;

#endif
