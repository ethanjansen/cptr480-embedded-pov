#include "DROTENC.h"

// other classes for initalization
#include "DGPIO.h"
#include "DTPM.h"
#include "DUART.h"

// Single instances
DGPIO g_gpio;
DTPM g_tpm;
DUART g_uart;

void DROTENC::init() {
    // initialize GPIO, TPM, and UART
    g_gpio.Init(); // TODO: set config for reset button. configure IRQHandlers
    g_tpm.init();  // TODO: set default config for TPM on RGB LED RED and BLUE
    g_uart.init(); // TODO: set default config for UART
}

void DROTENC::_changeTPM() {
    // TODO: implement,
    // define max position to match 100% duty cycle.

    // set the duty cycle of the TPM based on the position
    //g_tpm.setDutyCycle(_position);
}

void DROTENC::_sendPosition() {
    // TODO: implement.

}