#include "DDMA.h"
#include "DLEDS.h" // source, dest, callback for project
#include "MKL25Z4.h"

// static member initialization:

// configurations used in init
// MUST BE IN CHANNEL ORDER
const DDMA::DMAConfig DDMA::dmaConfigurations[] = {
//  {channel,             mode,                 incrementMode,           srcSize,               dstSize,               srcModulo,                 dstModulo,  enPeripheralRequest, triggerSource,              bytesToTransfer, src,  dst,      preHandler, postHandler}
    {DDMA::DMA_CHANNEL_0, DDMA::DMA_CYCLESTEAL, DDMA::DMA_INCREMENT_SRC, DDMA::DMA_BLOCKSIZE_8, DDMA::DMA_BLOCKSIZE_8, DDMA::DMA_MODULO_DISABLED, DDMA::DMA_MODULO_DISABLED, true, DDMA::DMAMUX_TPM2_CH1, 768, DLEDS::getDMABuf(), DLEDS::getDMADest(), DLEDS::startLEDReset, nullptr}, // Cycle Steal DMA from src to dst for 768 bytes (blocks of 16 LEDS with 48 bits each), TPM2 overflow triggered, preHandler callback to start PIT
};

// private variables
bool DDMA::_init;
DDMA::DMAInfo DDMA::_instances[DMA_NUM_CHANNELS]; // init to 0 (defaults to uninitialized)


// class methods:

// Initialize the DMA module.
// Reads initial configuration from dmaConfigurations[].
// Configures DMA mux and DMA module.
// Does not start.
void DDMA::init() {
    // Enable DMA/DMAMUX clocks
    SIM->SCGC7 |= SIM_SCGC7_DMA_MASK;
    SIM->SCGC6 |= SIM_SCGC6_DMAMUX_MASK;

    // Configure DMA Channels
    for (unsigned i = 0; i < sizeof(dmaConfigurations)/sizeof(DMAConfig); i++) {
        // Configure MUX trigger source
        DMAMUX0->CHCFG[i] = DMAMUX_CHCFG_ENBL_MASK | dmaConfigurations[i].triggerSource;

        // Set source and destination addresses
        _instances[i].src = dmaConfigurations[i].src; // could check if meets requirements, but am not going to
        _instances[i].dst = dmaConfigurations[i].dst;
        DMA0->DMA[i].SAR = (unsigned)dmaConfigurations[i].src;
        DMA0->DMA[i].DAR = (unsigned)dmaConfigurations[i].dst;

        // Set channel configuration
        // bytes to transfer
        DMA0->DMA[i].DSR_BCR = dmaConfigurations[i].bytesToTransfer & 0xFFFFF;
        // enable interrupts, cs/continuous mode, src/dst size, src/dst increment, src/dst modulo, and D_REQ based on enPeripheralRequest
        DMA0->DMA[i].DCR |= DMA_DCR_EINT_MASK | DMA_DCR_CS(dmaConfigurations[i].mode) 
                                                    | DMA_DCR_SSIZE(dmaConfigurations[i].srcSize) | DMA_DCR_DSIZE(dmaConfigurations[i].dstSize)
                                                    | DMA_DCR_SINC(dmaConfigurations[i].incrementMode) | DMA_DCR_DINC(dmaConfigurations[i].incrementMode >> 1)
                                                    | DMA_DCR_SMOD(dmaConfigurations[i].srcModulo) | DMA_DCR_DMOD(dmaConfigurations[i].dstModulo)
                                                    | DMA_DCR_D_REQ(dmaConfigurations[i].enPeripheralRequest);
        // Enable NVIC interrupts
        NVIC_EnableIRQ((IRQn)((unsigned)DMA0_IRQn + i));

        // Set running status to STOPPED, or RUNNING if enPeripheralRequest
        // if not enPeripheralRequest, DMA will not start until start() is called
        if (dmaConfigurations[i].enPeripheralRequest) {
            _instances[i].status = DMAStatus::DMA_RUNNING;
            DMA0->DMA[i].DCR |= DMA_DCR_ERQ_MASK;
        } else {
            _instances[i].status = DMAStatus::DMA_STOPPED;
        }
    }

    // Finish init
    _init = true;
}

// Start DMA channel.
// Does nothing if channel is already running (or unitialized).
// Does not reinitialize channel (or clear DONE).
void DDMA::start(DMAChannel channel) {
    if (_instances[channel].status != DMAStatus::DMA_STOPPED) {
        return;
    } else if (_instances[channel].status == DMAStatus::DMA_STOPPING) {
        _instances[channel].status = DMAStatus::DMA_RUNNING;
        return;
    }

    _instances[channel].status = DMAStatus::DMA_RUNNING;

    // Start DMA channel manually or by peripheral request
    if (dmaConfigurations[channel].enPeripheralRequest) { // assumes dmaConfigurations is properlly ordered
        DMA0->DMA[channel].DCR |= DMA_DCR_ERQ_MASK | DMA_DCR_D_REQ_MASK;
    } else {
        DMA0->DMA[channel].DCR |= DMA_DCR_START_MASK;
    }
}

// Stop DMA channel after transfer complete.
// Does nothing if channel is already stopped/stopping (or unitialized).
void DDMA::stop(DMAChannel channel) {
    if (_instances[channel].status == DMAStatus::DMA_RUNNING) {
        _instances[channel].status = DMAStatus::DMA_STOPPING;
    }
}

// Stop DMA channel immediately.
// Does nothing if channel is already stopped (or unitialized).
void DDMA::stopImmediately(DMAChannel channel) {
    if ((_instances[channel].status != DMAStatus::DMA_RUNNING) && (_instances[channel].status != DMAStatus::DMA_STOPPING)) {
        return;
    }
    _instances[channel].status = DMAStatus::DMA_STOPPED;
    DMA0->DMA[channel].DSR_BCR |= DMA_DSR_BCR_DONE_MASK;
}

// Check if DMA channel is busy (running/stopping).
// Returns false if stopped/uninitialized.
bool DDMA::isBusy(DMAChannel channel) {
    return (_instances[channel].status != DMAStatus::DMA_STOPPED) && (_instances[channel].status != DMAStatus::DMA_UNITIALIZED);
}

// Set channel source address.
// Must be properly aligned, not nullptr.
// Does nothing if uninitialized.
void DDMA::setSource(DMAChannel channel, const void *src) {
    if (_instances[channel].status == DMAStatus::DMA_UNITIALIZED) {
        return;
    }

    _instances[channel].src = src;
    DMA0->DMA[channel].SAR = (unsigned)src;
}

// Set channel destination address.
// Must be properly aligned, not nullptr.
// Does nothing if uninitialized.
void DDMA::setDestination(DMAChannel channel, void *dst) {
    if (_instances[channel].status == DMAStatus::DMA_UNITIALIZED) {
        return;
    }

    _instances[channel].dst = dst;
    DMA0->DMA[channel].DAR = (unsigned)dst;
}

// Get channel source address.
// Returns nullptr if uninitialized.
// See what address is currently being used as source, helpful to prevent memory issues.
const void *DDMA::getSource(DMAChannel channel) {
    if (_instances[channel].status == DMAStatus::DMA_UNITIALIZED) {
        return nullptr;
    }

    return _instances[channel].src;
}

// Get channel destination address.
// Returns nullptr if uninitialized.
// See what address is currently being used as destination, helpful to prevent memory issues.
void *DDMA::getDestination(DMAChannel channel) {
    if (_instances[channel].status == DMAStatus::DMA_UNITIALIZED) {
        return nullptr;
    }

    return _instances[channel].dst;
}


// Interrupt handler -- channel specific.
void DDMA::IRQHandler(DMAChannel channel) {
    // Check for error
    if (DMA0->DMA[channel].DSR_BCR & 0x70000000) { // CE/BES/BED
        while (true) { /* hang :( */}
    }

    // Call preHandler callback
    if (dmaConfigurations[channel].preHandler) {
        dmaConfigurations[channel].preHandler();
    }

    // Handle DMA configuration:

    // Clear flags
    DMA0->DMA[channel].DSR_BCR |= DMA_DSR_BCR_DONE_MASK;
    // Reset BCR, src, dst
    DMA0->DMA[channel].DSR_BCR = dmaConfigurations[channel].bytesToTransfer;
    DMA0->DMA[channel].SAR = (unsigned)_instances[channel].src;
    DMA0->DMA[channel].DAR = (unsigned)_instances[channel].dst;
    // Restart DMA if not stopping and enPeripheralRequest
    if ((_instances[channel].status == DMAStatus::DMA_RUNNING) && dmaConfigurations[channel].enPeripheralRequest) {
        DMA0->DMA[channel].DCR |= DMA_DCR_ERQ_MASK | DMA_DCR_D_REQ_MASK;
    } else {
        _instances[channel].status = DMAStatus::DMA_STOPPED;
    }

    // Call postHandler callback
    if (dmaConfigurations[channel].postHandler) {
        dmaConfigurations[channel].postHandler();
    }
}