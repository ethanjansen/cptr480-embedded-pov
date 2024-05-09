#include "DTPM.h"
#include "MKL25Z4.h"

// Initial Configurations
// Note: The class functions will catch errors arising for improper prescale divisors, frequencies, and duty cycles
// (some combinations are not possible due to 16bit limitation). It is your job to ensure that the prescale divisor 
// will suite your frequency and dutyCycle needs.
const DTPM::TPMConfig DTPM::tpmConfigs[] = {
//  {TPMName,     TPMChannel, TPMPrescaleDivisor, PWMMode,        PWMPolarity, frequency, dutyCyclePercent }
    {DTPM::TPM_2, DTPM::TPM_CH0, DTPM::DIV_4, DTPM::EDGE_ALIGNED, DTPM::HIGH_ON_MATCH, 184, 0},  // Red
//    {DTPM::TPM_2, DTPM::TPM_CH1, DTPM::DIV_4, DTPM::EDGE_ALIGNED, DTPM::HIGH_ON_MATCH, 184, 0},     // Green -- unused
    {DTPM::TPM_0, DTPM::TPM_CH1, DTPM::DIV_4, DTPM::EDGE_ALIGNED, DTPM::HIGH_ON_MATCH, 184, 0},     // Blue
};

// Static var initialization
bool DTPM::_init;

// nothing initialized
unsigned DTPM::_tpmInitialized[NUM_TPMS];

// Initialize TPM module.
// No interrupts enabled...
// returns 1 on failure, 0 on success.
unsigned DTPM::init() {
    // Enable TPM clock source
    SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1) | SIM_SOPT2_PLLFLLSEL_MASK;

    for (unsigned i = 0; i < sizeof(tpmConfigs)/sizeof(TPMConfig); i++) {
        // this could likely be done with some math to dynamically calculate the TPM_x addresses rather than using a switch...
        switch (tpmConfigs[i].tpm) {
            case TPM_0:
                if (!_tpmInitialized[TPM_0]) { // check if module is unitialized or MOD is 0
                    // module-level stuff
                    // Enable TPM clock
                    SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;
                    // Set PWM mode, clock enable, and prescale divisior
                    TPM0->SC |= TPM_SC_CPWMS(tpmConfigs[i].pwmMode) | TPM_SC_CMOD(1) | TPM_SC_PS(tpmConfigs[i].prescalerDivisor);
                    // Set MOD value
                    signed mod = setFrequency(tpmConfigs[i].tpm, tpmConfigs[i].frequency, tpmConfigs[i].prescalerDivisor, tpmConfigs[i].pwmMode);
                    if (mod > 0) {
                        _tpmInitialized[TPM_0] = mod;
                    } else {
                        return 1; // invalid frequency given prescale Divisor and pwmMode.
                    }
                }

                // channel-level stuff
                if (tpmConfigs[i].channel >= NUM_TPM0CHANNELS) {
                    return 1; // invalid channel
                }
                // Set channel TPM to PWM and set polarity
                TPM0->CONTROLS[tpmConfigs[i].channel].CnSC |= TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA(tpmConfigs[i].pwmPolarity);
                // Set channel duty cycle
                if (setDutyCycle(tpmConfigs[i].tpm, tpmConfigs[i].channel, tpmConfigs[i].dutyCyclePercent, tpmConfigs[i].pwmMode) < 0) {
                    return 1; // invalid duty cycle given frequency
                }
                continue;
            case TPM_1:
                if (!_tpmInitialized[TPM_1]) { // check if module is unitialized or MOD is 0
                    // module-level stuff
                    // Enable TPM clock
                    SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;
                    // Set PWM mode, clock enable, and prescale divisior
                    TPM1->SC |= TPM_SC_CPWMS(tpmConfigs[i].pwmMode) | TPM_SC_CMOD(1) | TPM_SC_PS(tpmConfigs[i].prescalerDivisor);
                    // Set MOD value
                    signed mod = setFrequency(tpmConfigs[i].tpm, tpmConfigs[i].frequency, tpmConfigs[i].prescalerDivisor, tpmConfigs[i].pwmMode);
                    if (mod > 0) {
                        _tpmInitialized[TPM_1] = mod;
                    } else {
                        return 1; // invalid frequency given prescale Divisor and pwmMode.
                    }
                }

                // channel-level stuff
                if (tpmConfigs[i].channel >= NUM_TPM1CHANNELS) {
                    return 1; // invalid channel
                }
                // Set channel TPM to PWM and set polarity
                TPM1->CONTROLS[tpmConfigs[i].channel].CnSC |= TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA(tpmConfigs[i].pwmPolarity);
                // Set channel duty cycle
                if (setDutyCycle(tpmConfigs[i].tpm, tpmConfigs[i].channel, tpmConfigs[i].dutyCyclePercent, tpmConfigs[i].pwmMode) < 0) {
                    return 1; // invalid duty cycle given frequency
                }
                continue;
            case TPM_2:
                if (!_tpmInitialized[TPM_2]) { // check if module is unitialized or MOD is 0
                    // module-level stuff
                    // Enable TPM clock
                    SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK;
                    // Set PWM mode, clock enable, and prescale divisior
                    TPM2->SC |= TPM_SC_CPWMS(tpmConfigs[i].pwmMode) | TPM_SC_CMOD(1) | TPM_SC_PS(tpmConfigs[i].prescalerDivisor);
                    // Set MOD value
                    signed mod = setFrequency(tpmConfigs[i].tpm, tpmConfigs[i].frequency, tpmConfigs[i].prescalerDivisor, tpmConfigs[i].pwmMode);
                    if (mod > 0) {
                        _tpmInitialized[TPM_2] = mod;
                    } else {
                        return 1; // invalid frequency given prescale Divisor and pwmMode.
                    }
                }

                // channel-level stuff
                if (tpmConfigs[i].channel >= NUM_TPM2CHANNELS) {
                    return 1; // invalid channel
                }
                // Set channel TPM to PWM and set polarity
                TPM2->CONTROLS[tpmConfigs[i].channel].CnSC |= TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK | TPM_CnSC_ELSA(tpmConfigs[i].pwmPolarity);
                // Set channel duty cycle
                if (setDutyCycle(tpmConfigs[i].tpm, tpmConfigs[i].channel, tpmConfigs[i].dutyCyclePercent, tpmConfigs[i].pwmMode) < 0) {
                    return 1; // invalid duty cycle given frequency
                }
                continue;
            default:
                continue; // should not get here if tpmConfigs is correct
        }
    }

    // successful init
    _init = true;
    return 0;
}

// Set PWM duty cycle.
// expects dutyCyclePercent to be between 0 and 10000 (0-100.00%).
// Will set duty Cycle to 100-dutyCyclePercent for "High on match" PWM polarity.
// returns channel CnV on success, negative on failure
signed DTPM::setDutyCycle(TPMName tpm, TPMChannel channel, unsigned dutyCyclePercent, PWMMode pwmMode) {
    // check proper inputs and compute duty cycle:
    // Edge-aligned: CnV/(MOD+1) [CnV = (percent * (MOD+1))/10000]
        // Low on match: CnV/(MOD+1)
        // High on match: (MOD+1-CnV)/(MOD+1) = 1 - CnV/(MOD+1)
    // Center-aligned: CnV/MOD [CnV = (percent * MOD)/10000]
    unsigned cnv;
    if (dutyCyclePercent > 10000 || _tpmInitialized[tpm] == 0) {
        return -1;
    } else if (dutyCyclePercent == 10000) {
        cnv = 0xffff;
    } else {
        if (pwmMode == EDGE_ALIGNED) {
            cnv = (dutyCyclePercent * (_tpmInitialized[tpm]+1))/10000;
        } else {
            cnv = (dutyCyclePercent * _tpmInitialized[tpm])/10000;
        }
    }

    // set CnV
    return setCnV(tpm, channel, cnv);
}

// Set PWM CnV directly
// expects cnv between 0 and 65535
// returns channel CnV on success, negative on failure.
signed DTPM::setCnV(TPMName tpm, TPMChannel channel, unsigned cnv) {
    if (cnv > 0xffff) {
        return -1;
    }
    
    // set CnV -- check channel
    switch (tpm) {
        case TPM_0:
            if (channel > NUM_TPM0CHANNELS) {
                return -1;
            }
            TPM0->CONTROLS[channel].CnV = cnv;
            break;
        case TPM_1:
            if (channel > NUM_TPM1CHANNELS) {
                return -1;
            }
            TPM1->CONTROLS[channel].CnV = cnv;
            break;
        case TPM_2:
            if (channel > NUM_TPM2CHANNELS) {
                return -1;
            }
            TPM2->CONTROLS[channel].CnV = cnv;
            break;
        default:
            return -1; // bad module
    }

    return cnv;
}


// Set PWM frequency.
// Configures module MOD from freq, prescaleDivisor and pwmMode.
// returns module MOD on success, non-positive on failure
signed DTPM::setFrequency(TPMName tpm, unsigned freq, TPMPrescaleDivisor prescalerDivisor, PWMMode pwmMode) {
    // calculate frequency
    // Edge aligned: frequency = 48MHz/prescalerDivisor/(MOD+1) [MOD = 48E6/(frequency*prescalerDivisor) - 1]
    // Center aligned: frequency = 48MHz/prescalerDivisor/2/MOD [MOD = 48E6/(frequency*prescalerDivisor*2)]
    unsigned mod;
    if (pwmMode == EDGE_ALIGNED) {
        mod = (48000000/(freq * (1 << prescalerDivisor))) - 1;
        if (mod > 0xFFFF) {
            return -1;
        }
    } else {
        mod = 48000000/(freq * (1 << prescalerDivisor) * 2);
        if ((mod == 0) || (mod > 0x7FFF)) {
            return -1;
        }
    }

    // set mod
    switch (tpm) {
        case TPM_0:
            TPM0->MOD = mod;
            break;
        case TPM_1:
            TPM1->MOD = mod;
            break;
        case TPM_2:
            TPM2->MOD = mod;
            break;
        default:
            return -1; // invalid tpm
    }

    return mod;
}


// Start PWM.
// Just changes module CMOD.
void DTPM::start(TPMName tpm) {
    switch (tpm) {
        case TPM_0:
            TPM0->SC |= TPM_SC_CMOD(1);
            return;
        case TPM_1:
            TPM1->SC |= TPM_SC_CMOD(1);
            return;
        case TPM_2:
            TPM2->SC |= TPM_SC_CMOD(1);
            return;
        default:
            return;
    }
}

// Stop PWM channel.
// Just changes module CMOD.
void DTPM::stop(TPMName tpm) {
    switch (tpm) {
        case TPM_0:
            TPM0->SC &= ~TPM_SC_CMOD(3);
            return;
        case TPM_1:
            TPM1->SC &= ~TPM_SC_CMOD(3);
            return;
        case TPM_2:
            TPM2->SC &= ~TPM_SC_CMOD(3);
            return;
        default:
            return;
    }
}

// interrupt handler.
// currently hangs the system -- don't enable.
void DTPM::IRQHandler() {
    while (1) { /* hang */}
}
