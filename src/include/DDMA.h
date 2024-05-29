#ifndef DDMA_H
#define DDMA_H

// Driver for DMA module.
// No support for channel linking, memory restriction checking (to improve speed), or PIT trigger support.
// Does not have internal buffer, but rather uses global pointers to reference source and destination (src/dst).
// Requires peripheral triggers to be enabled separately (currently TPM supports this).
// Note: This driver is WIP and not fully tested; it does work well enough for Lab08 however.
class DDMA {
    public:
        // enums:

        enum DMAChannel {
            DMA_CHANNEL_0,
            DMA_CHANNEL_1,
            DMA_CHANNEL_2,
            DMA_CHANNEL_3,
            DMA_NUM_CHANNELS
        };

        enum DMAMode {
            DMA_CONTINUOUS,
            DMA_CYCLESTEAL
        };

        enum DMAIncrement { // bit 0 for source, bit 1 for destination
            DMA_NO_INCREMENT,
            DMA_INCREMENT_SRC,
            DMA_INCREMENT_DST,
            DMA_INCREMENT_BOTH
        };

        // used for both source and destination
        enum DMABlockSize { // in bits
            DMA_BLOCKSIZE_32,
            DMA_BLOCKSIZE_8,
            DMA_BLOCKSIZE_16
        };

        // used for both source and destination
        enum DMAAddressModulo { // in bytes
            DMA_MODULO_DISABLED,
            DMA_MODULO_16,
            DMA_MODULO_32,
            DMA_MODULO_64,
            DMA_MODULO_128,
            DMA_MODULO_256,
            DMA_MODULO_512,
            DMA_MODULO_1K,
            DMA_MODULO_2K,
            DMA_MODULO_4K,
            DMA_MODULO_8K,
            DMA_MODULO_16K,
            DMA_MODULO_32K,
            DMA_MODULO_64K,
            DMA_MODULO_128K,
            DMA_MODULO_256K
        };

        // no PIT support
        enum DMAMUXTriggerSource { // set DMAMUX
            DMAMUX_CHANNEL_DISABLED,
            DMAMUX_UART0_RX = 2,
            DMAMUX_UART0_TX,
            DMAMUX_UART1_RX,
            DMAMUX_UART1_TX,
            DMAMUX_UART2_RX,
            DMAMUX_UART2_TX,
            DMAMUX_SPI0_RX = 16,
            DMAMUX_SPI0_TX,
            DMAMUX_SPI1_RX,
            DMAMUX_SPI1_TX,
            DMAMUX_I2C0 = 22,
            DMAMUX_I2C1,
            DMAMUX_TPM0_CH0,
            DMAMUX_TPM0_CH1,
            DMAMUX_TPM0_CH2,
            DMAMUX_TPM0_CH3,
            DMAMUX_TPM0_CH4,
            DMAMUX_TPM0_CH5,
            DMAMUX_TPM1_CH0 = 32,
            DMAMUX_TPM1_CH1,
            DMAMUX_TPM2_CH0,
            DMAMUX_TPM2_CH1,
            DMAMUX_ADC0 = 40,
            DMAMUX_CMP0 = 42,
            DMAMUX_DAC0 = 45,
            DMAMUX_PORTA = 49,
            DMAMUX_PORTD = 52,
            DMAMUX_TPM0_OVERFLOW = 54,
            DMAMUX_TPM1_OVERFLOW,
            DMAMUX_TPM2_OVERFLOW,
            DMAMUX_TSI,
            DMAMUX_ALWAYS_ENABLED = 60
        };

        // Interrupt handler callbacks
        // Optional/extra. Not used to handle DMA configuration (can be used to stop DMA however)
        typedef void (*DMAInterruptHandler)();

        // configuration struct
        struct DMAConfig {
            DMAChannel channel : 3;
            DMAMode mode : 1; // continuous or cycle steal
            DMAIncrement incrementMode : 2; // sets source and destination increments
            DMABlockSize srcSize : 2;
            DMABlockSize dstSize : 2;
            DMAAddressModulo srcModulo : 4;
            DMAAddressModulo dstModulo : 4;
            bool enPeripheralRequest : 1; // allow peripheral to initiate transfer (set START) (sets ERQ and D_REQ)
            DMAMUXTriggerSource triggerSource : 6; // set DMAMUX
            unsigned bytesToTransfer : 20; // 1 to 0x0FFFFF
            const void *src; // source address
            void *dst; // destination address
            DMAInterruptHandler preHandler; // called in IRQHandler first
            DMAInterruptHandler postHandler; // called in IRQHandler last
        };

        // configurations used in init
        // MUST BE IN CHANNEL ORDER
        static const DMAConfig dmaConfigurations[];

        // class methods:

        // Initialize the DMA module.
        // Reads initial configuration from dmaConfigurations[].
        // Configures DMA mux and DMA module.
        // Does not start.
        static void init();

        // Check driver initialization status.
        static inline bool isInit() { return _init; }

        // Start DMA channel.
        // Does nothing if channel is already running (or unitialized).
        static void start(DMAChannel channel);

        // Stop DMA channel after transfer complete.
        // Does nothing if channel is already stopped/stopping (or unitialized).
        static void stop(DMAChannel channel);

        // Stop DMA channel immediately.
        // Does nothing if channel is already stopped (or unitialized).
        static void stopImmediately(DMAChannel channel);

        // Check if DMA channel is busy (running/stopping).
        // Returns false if stopped/uninitialized.
        static bool isBusy(DMAChannel channel);

        // Set channel source address.
        // Must be properly aligned, not nullptr.
        // Does nothing if uninitialized.
        static void setSource(DMAChannel channel, const void *src);

        // Set channel destination address.
        // Must be properly aligned, not nullptr.
        // Does nothing if uninitialized.
        static void setDestination(DMAChannel channel, void *dst);

        // Get channel source address.
        // Returns nullptr if uninitialized.
        // See what address is currently being used as source, helpful to prevent memory issues.
        static const void *getSource(DMAChannel channel);

        // Get channel destination address.
        // Returns nullptr if uninitialized.
        // See what address is currently being used as destination, helpful to prevent memory issues.
        static void *getDestination(DMAChannel channel);

        // Other configurations are not likely to change as dynamically, thus configure with init.

        // Interrupt handler -- channel specific.
        static void IRQHandler(DMAChannel channel);

    private:
        // no copy assignment, or constructor.
        DDMA();
        DDMA(const DDMA&);
        void operator=(const DDMA&);

        // private information

        enum DMAStatus {
            DMA_UNITIALIZED,
            DMA_RUNNING,
            DMA_STOPPED,
            DMA_STOPPING
        };

        struct DMAInfo { // also store interrupt handlers for fast lookup
            DMAStatus status;
            const void *src;
            void *dst;
        };

        // private vars

        // holds channel running status and source/destination addresses
        static DMAInfo _instances[DMA_NUM_CHANNELS];

        // private vars
        static bool _init;

};


#endif // DDMA_H