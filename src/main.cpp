#include "DDMA.h"
#include "DGPIO.h"
#include "DTPM.h"
#include "MKL25Z4.h"

// Globals for DMA
const unsigned char g_waveform[16] __attribute__((aligned(16))) = {3, 2, 1, 0, 25, 24, 29, 28, 9, 8, 13, 12, 9, 8, 25, 24}; // fancy values to match required Lab08 waveform.
const unsigned char *g_dmaSrcData = g_waveform;
const void *g_dmaDstData = DGPIO::getOutputRegister(DGPIO::WAVE0);

int main() {
    // Initialize GPIO, TPM, and DMA.
    // No need to check if already initialized, this only happens here for this simple lab.
    DGPIO::init();
    DTPM::init();
    DDMA::init();

    while (1)
    {
        // mining bitcoin...
    }
} 


