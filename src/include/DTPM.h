#ifndef DTPM_H
#define DTPM_H

// Driver for TPM module.
// Only enables edge-aligned PWM output.
class DTPM {
    public:
        // enums.
        // TODO: label defaults

        // TPM Names.
        enum TPMName {
            TPM_0, // 6 channel.
            TPM_1, // 2 channel.
            TPM_2, // 2 channel.
            NUM_TPMS,
        };

        // TPM Channels.
        enum TPMChannel {
            TPM_CH0,
            TPM_CH1,
            TPM_CH2, // only for TPM_0
            TPM_CH3, // only for TPM_0
            TPM_CH4, // only for TPM_0
            TPM_CH5, // only for TPM_0
            NUM_TPM0CHANNELS = 6,
            NUM_TPM1CHANNELS = 2,
            NUM_TPM2CHANNELS = 2,
        };

        // TPM Clock prescaler divisor
        enum TPMPrescaleDivisor {
            DIV_1, // default
            DIV_2,
            DIV_4,
            DIV_8,
            DIV_16,
            DIV_32,
            DIV_64,
            DIV_128,
        };

        // PWM Modes
        enum PWMMode {
            EDGE_ALIGNED,
            CENTER_ALIGNED,
        };

        // PWM Polarity
        enum PWMPolarity {
            LOW_ON_MATCH, // 0 on match, 1 on reload
            HIGH_ON_MATCH, // 1 on match, 0 on reload
        };

        // TPM Configuration struct.
        struct TPMConfig {
            TPMName tpm;
            TPMChannel channel;
            TPMPrescaleDivisor prescalerDivisor;
            PWMMode pwmMode;
            PWMPolarity pwmPolarity;
            unsigned frequency;
            unsigned dutyCyclePercent;
        };

        // TPM initial module configuration
        // set before calling init()
        // constant for compile-time configuration
        static const TPMConfig tpmConfigs[];

        // empty constructor.
        DTPM();

        // TPM Initialization API.
        unsigned init();

        // Set PWM duty cycle.
        // expects dutyCyclePercent to be between 0 and 100.
        // returns channel CnV on success, negative on failure
        signed setDutyCycle(TPMName tpm, TPMChannel channel, unsigned dutyCyclePercent, PWMMode pwmMode);

        // Set PWM CnV directly
        // expects cnv between 0 and 65535
        // returns channel CnV on success, negative on failure.
        signed setCnV(TPMName tpm, TPMChannel channel, unsigned cnv);

        // Configures module MOD from freq, prescaleDivisor and pwmMode.
        // returns module MOD on success, non-positive on failure
        signed setFrequency(TPMName tpm, unsigned freq, TPMPrescaleDivisor prescalerDivisor, PWMMode pwmMode);

        // Start PWM module.
        // can control individual channel via setDutyCycle(dutyCyclePercent=%)
        void start(TPMName tpm);

        // Stop PWM module.
        // can control individual channel via setDutyCycle(dutyCyclePercent=0)
        void stop(TPMName tpm);

        // interrupt handler.
        // currently hangs the system -- don't enable.
        void IRQHandler();

    private:
        // no copy or assignment.
        DTPM(const DTPM&);
        void operator=(const DTPM&);

        // Status flags for if each TPM is initialized. Holds the TPM MOD value.
        // This prevents multiple frequencies, modes, and prescalerDivisors from being set on the same module.
        static unsigned _tpmInitialized[NUM_TPMS];
};

// single global instance.
extern DTPM g_tpm;

#endif // DTPM_H