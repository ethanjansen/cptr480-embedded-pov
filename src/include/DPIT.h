#ifndef DPIT_H
#define DPIT_H

// PIT Driver
class DPIT {
    public:
        // PIT Names
        enum PITName {
            PIT0,
            PIT1,
            NUM_PITNAMES
        };

        // holds interval for each PIT. Indexed by PITName
        static unsigned pitIntervals[NUM_PITNAMES];

        // Initialize PIT clock, interrupts, and enable PIT
        static void init();

        // Check initialization status
        static inline bool isInit() { return _init; }

        // Set PIT interval (interrupts per second)
        static void setInterruptsPerSec(PITName pit, unsigned interruptsPerSec);

        // Set PIT interval (seconds per interrupt)
        static void setSecPerInterrupt(PITName pit, unsigned secPerInterrupt);

        // Stop PIT
        static void stop(PITName pit); // add enum to select which PIT? -- return code?

        // Start PIT
        static void start(PITName pit); // add enum to select which PIT? -- return code?

        // Partially deterministic sleep.
        // automatically sets "pit" PIT interval based on "ms", starts and stops "pit".
        // This is blocking.
        static void sleep(PITName pit, unsigned ms);

        // FUTURE WORK:
        // Start PIT chaining PIT 0 and 1 for 64bit timer
        // specify interval timer
        // void startChained(unsigned long interval); // check if other timers running? -- return code?

        // PIT interrupt handler
        static void IRQHandler();

    private:
        // no copy or assignment
        DPIT();
        DPIT(const DPIT&);
        void operator=(const DPIT&);

        // private data
        static bool _init;

        // blocking flag
        static bool _block[NUM_PITNAMES];
};

#endif