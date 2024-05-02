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

        // empty constructor
        DPIT();

        // Initialize PIT clock, interrupts, and enable PIT
        void init();

        // Set PIT interval (interrupts per second)
        void setInterruptsPerSec(PITName pit, unsigned interruptsPerSec);

        // Set PIT interval (seconds per interrupt)
        void setSecPerInterrupt(PITName pit, unsigned secPerInterrupt);

        // Stop PIT
        void stop(PITName pit); // add enum to select which PIT? -- return code?

        // Start PIT
        void start(PITName pit); // add enum to select which PIT? -- return code?

        // FUTURE WORK:
        // Start PIT chaining PIT 0 and 1 for 64bit timer
        // specify interval timer
        // void startChained(unsigned long interval); // check if other timers running? -- return code?

        // PIT interrupt handler
        void IRQHandler();

    private:
        // no copy or assignment
        DPIT(const DPIT&);
        void operator=(const DPIT&);

        // I want to have the ability to initialize 2 PITs separately.
        // I want to start them separately and have them interrupt independently at different times.
        
};

// single global instance
extern DPIT g_pit;

#endif