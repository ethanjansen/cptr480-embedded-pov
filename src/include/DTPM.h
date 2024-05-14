#ifndef DTPM_H
#define DTPM_H

// Driver for TPM module.
// Supports edge-aligned and center-aligned PWM modes, as well as input capture mode
class DTPM {
    public:
        // enums.

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
        // TOOD: check values
        enum TPMMode {
            PWM_EDGE_ALIGNED,
            PWM_CENTER_ALIGNED,
            INPUT_CAPTURE,
        };

        // Granular mode settings - PWM polary and input capture edge
        // TOOD: check values
        enum TPMModeSetting {
            PWM_LOW_ON_MATCH, // PWM: 0 on match, 1 on reload
            PWM_HIGH_ON_MATCH, // PWM: 1 on match, 0 on reload
            CAPTURE_RISING_EDGE, // Input Capture: capture on rising edge
            CAPTURE_FALLING_EDGE, // Input Capture: capture on falling edge
            CAPTURE_BOTH_EDGES, // Input Capture: capture on both edges
        };

        // Interrupt Handler callback
        // This is optional, interrupt flags will be cleared before this is called
        typedef void (*interruptCallback)(void);

        // TPM Configuration struct.
        struct TPMConfig {
            TPMName tpm;
            TPMChannel channel;
            TPMPrescaleDivisor prescalerDivisor;
            TPMMode tpmMode;
            TPMModeSetting tpmModeSetting;
            unsigned frequency;
            unsigned dutyCyclePercent; // ignored for input capture mode
            interruptCallback overflowInterrupt; // applies to all channels on TPM_x
            interruptCallback channelInterrupt; // TPM_x channel specific interrupt
        };

        // TPM initial module configuration
        // set before calling init()
        // constant for compile-time configuration
        static const TPMConfig tpmConfigs[];

        // TPM Initialization API.
        static unsigned init();

        // Check initialization status.
        static inline bool isInit() { return _init; }

        // Set PWM duty cycle.
        // expects dutyCyclePercent to be between 0 and 10000 (0-100.00%).
        // This is not applicable in input capture mode.
        // returns channel CnV on success, negative on failure.
        static signed setDutyCycle(TPMName tpm, TPMChannel channel, unsigned dutyCyclePercent, TPMMode pwmMode);

        // Set PWM CnV directly.
        // expects cnv between 0 and 65535.
        // This is not applicable in input capture mode.
        // returns channel CnV on success, negative on failure.
        static signed setCnV(TPMName tpm, TPMChannel channel, unsigned cnv);

        // Configures module MOD from freq, prescaleDivisor and pwmMode.
        // returns module MOD on success, non-positive on failure
        static signed setFrequency(TPMName tpm, unsigned freq, TPMPrescaleDivisor prescalerDivisor, TPMMode tpmMode);

        // Start PWM module.
        // can control individual channel via setDutyCycle(dutyCyclePercent=%) (in PWM mode).
        static void start(TPMName tpm);

        // Stop PWM module.
        // can control individual channel via setDutyCycle(dutyCyclePercent=0) (in PWM mode).
        static void stop(TPMName tpm);

        // interrupt handler.
        // Checks TPM channels in tpmConfigs for interrupt flags, clears flags, and calls interruptCallback.
        static void IRQHandler();

    private:
        // no copy or assignment.
        DTPM();
        DTPM(const DTPM&);
        void operator=(const DTPM&);

        // Status flags for if each TPM is initialized. Holds the TPM MOD value.
        // This prevents multiple frequencies, modes, and prescalerDivisors from being set on the same module.
        static unsigned _tpmInitialized[NUM_TPMS];

        // Status flag for if the TPM module is initialized.
        static bool _init;
};

#endif // DTPM_H