# CPTR480 Labs & Projects

## [Lab 01:](https://gitlab.cs.wallawalla.edu/janset/student480/-/tree/Lab01?ref_type=tags)
 - Question 2 Answer:
   > The PIT is still running based on the peripheral register view. After getting to the end of the handler via step over/step into the debugger loses all information (about variables, the stack, registers, etc.) but the process is still paused somewhere outside of what is visible. I watched that ```g_pitTicks``` is incremented (as expected) after every continue. When watching a random ```int``` that I added to main it behaved as expected but I was unable to watch it while I was in some other scope--such as when inside the PIT interrupt handler.
 - Question 4 Answer:
   > The debugger behaved as expected (after realizing that I had to stop it, build, and start it again for changes to take affect). It was very useful for finding the values a variables. For instance, it helped me realize that I had 'S' and 'T' swapped in my morse code LUT. I did not use it to check what was actually going to be sent (via ```readNextMorseStringBit()```) to the GPIO, but rather check ahead of time the array that would eventually be output--easier than fighting with the PIT timing.

## [Lab 02:](https://gitlab.cs.wallawalla.edu/janset/student480/-/tree/Lab02?ref_type=tags)

## [Lab 03:](https://gitlab.cs.wallawalla.edu/janset/student480/-/tree/Lab03?ref_type=tags)
 - Step 1 Computations:
   > - Duty Cycle:
   >   - Edge-aligned: ```CnV/(MOD+1)```
   >   - Center-aligned: ```CnV/MOD```
   > - Frequency:
   >   - Edge-aligned: ```48MHz/prescalerDivisor/(MOD+1)```
   >   - Center-aligned: ```48MHz/prescalerDivisor/2/MOD```
 - Question 2 Answers:
   > Can make the high frequency noise basically disappear on the PWM DAC implementation if the PWM frequency is high enough. The noise is still quite apparent at 1kHz, but at 10kHz it is only about 20mV pk-pk. At a frequency of 100kHz can't discern the noise besides the edge-effects which remain at many mV. However, increasing the frequency has the drawback of reducing the voltage resolution. This is because higher frequencies require lower ```MOD``` values, which limit the range of ```CnV``` values that do not correspond to 100% duty cycle.
 - 3 LED Flicker Notes:
   > In order to have as much resolution as possible I want to fill (or close to it) the ```MOD``` register, but I will change the ```prescaleDivisor``` as slow as possible while still not seeing any flicker. At about 92Hz the flicker is gone unless I move it around, then I might be able to see it. At about 184Hz it is completly gone. I will aim for 184Hz with a ```prescaleDivisor``` of 4.
 - Question 5 Answer:
   > I found that a PIT interrupt frequency from 10-20Hz appears to cause smooth fading. I will choose 17Hz (it is quite smooth, but doesn't cycle too fast).
 - Final Implementation Notes:
   > Originally I had created a TPM driver where I could change the ```CnV``` value by specifying a duty cycle "percentage" from 0-10000 (0-100.00%). While this mostly worked with an exponential fade, it did not work well with the squaring implementation of gamma correction. In order to address this I added a way of setting ```CnV``` directly with the TPM driver. Also I decided to keep the RED LED pulsing effect as an option, but I (for some reason) preferred the exponential effect rather than the (seemingly) linear transition--the exponential effect was too fast for color cycling. Because of this my ```LED``` class supports both exponential pulsing and linear cycling.

## [Lab 04:](https://gitlab.cs.wallawalla.edu/janset/student480/-/tree/Lab04?ref_type=tags)
  - Design Decisions: Step by Step:
   > **```DGPIO```:** In the last lab I had extended ```DGPIO``` to handle alternate pin muxing modes. This lab I extended on this to enable interrupt modes. Now, ```DGPIO``` has its generic handler which will check which pin caused the interrupt (it keeps a list of interruptable pins), calling the appropriate handler (via callbacks), and finally clearing flags. Using callbacks was a fun idea but a bit of a headache as it does require static class functions (or, generally, functions not belonging to an object instance). Now that I know this, this shouldn't be too bad; and I like the flexibility.

   > **```DTPM```:** My previous implementation needed no changes for this lab, besides not enabling the green LED and initializing everything to 0% duty cycle.

   > **```DUART```:** My previous implementation was mostly good enough, however I added a new method ```sendInt()``` which uses ```int2str()``` to send an ```int``` as ASCII over UART0. I did refactor ```int2str()``` a bit to remove the library includes and to include null termination. In general I like simply null termination for this, but for this particular use I do a bit of extra manipulation to terminate with "\n\0".

   > **```DROTENC```:** This class is new to handle the rotary encoder. It uses an internal state machine to handle the position changes, as well as includes increment, decrement, control, reset, and get methods. It also prints the current position at each change via UART0. This class is also a static class, in that all methods are static class methods and that an instance cannot be created. I consider this to be "proper" as there is only one rotary encoder, plus this better supports the callback restrictions previously mentioned. Thus there is no global ```g_rotenc``` instance that needs to be made, everything happens directly on the class.

  - Challenges, Bugs, and Solutions:
   > For this lab everything worked *exactly* according to my code. This includes the ```HardFaults``` that were generated by enabling interrupts on the port pins. This happened because I initially enabled interrupts on the pins before the rest of the GPIO configuration (PU/PD, In/Out) which led to an interrupt being generated on ```init()``` even after clearing the ```ISF``` registers, which led to a handler being called which expected other peripherals to be clocked that were not yet. I addressed this by enabling pin interrupts after the rest of the GPIO configuration in ```DGPIO::init()```, and by initializing ```g_gpio``` after the other peripherals.

   > Another challenge I ran into was the use of callbacks for interrupt handlers. It is great in theory, however instance methods cannot be used (easily) with callbacks. The best solution was to use static methods, however I did not realize this until I had already designed the whole class which had the handlers reference other instance methods (Which they could not do if static). My solution to this was to make the entire ```DROTENC``` class static -- as discussed in the 'Design Decisions' section above.

  - For some fun with the LED Bar checkout [3-fun](https://gitlab.cs.wallawalla.edu/janset/student480/-/tree/3-fun?ref_type=heads). Note that this is a very rough implementation that I did on a whim. Also requires ~2.5A externally.

## [Lab 05:](https://gitlab.cs.wallawalla.edu/janset/student480/-/tree/Lab05?ref_type=tags)
  - Design Notes:
   > Now all classes are static with ```_init``` flags so that each peripheral could be initialized in different places without needing to worry about accidentally doing the same initialization twice. ```DGPIO``` is again used to pin muxing, now with SPI1 pins to the LSM6DSL. The new ```DSPI``` driver class handles SPI generically in master, 4-wire mode with Tx/Rx support of up to 13 bytes (for future use of reading accelerometer/gyro values). ```DMOTION``` utilizes the ```DSPI``` class with setup (coming soon) for the LSM6DSL and basic ```_readReg()```(with support for reading multiple registers sequentially (when "rounding" is enabled)) and ```_writeReg``` private functions. Currently functionality only supports ```probe()``` (implemented as blocking) to check for the "whoami" register, however the classes have been designed with future functionality in mind (see Lab 06). Also, I implemented a ```sleep()``` function with ```DPIT``` in order to wait ~15ms for the LSM6DSL to power up before reading/writing to it.

  - Challenges, Bugs, Solutions, and Working with the Logic Analyzer:
   > Because I wanted a generic ```DSPI``` class, I wanted to be able to support both SPI1 and SPI0 (even though that is unnecessary for this lab), this created a bit of an issue with baud rates because the input clock is different between the two modules. In order to address this the driver supports some predefined baud rates which can be easily adjusted at runtime to support the different SPI modules.

   > In the past I have used switch statements in order to initialize different modules of the same peripheral (such as TPM1 vs TPM2). This time I tried to avoid switch statements by calculating the correct register address ahead of time and then using that to initialize/configure/read/write/etc. This created some issues however as originally I was using a peripheral pointer type (such as ```SPI_Type *```) and adding an ```unsigned int``` to it, however this did not perform the addition as I was expected. I realized that I needed to use one of the peripheral "bases" and then typecast later in order for the math to be performed correctly. This issue originally prevented any modules from being configured, and took a bunch of debug stepping and test variables to figure out.

   > Once I got the SPI peripheral enabled I was getting unsuccessful probes. This was addressed with the logic analyzer (once I figured out to only take ~50 samples at 2MHz using the SPI protocol setting). I saw that my SPI modes were correct, and I was indeed sending a read command to the "whoami" address, however I was not clocking long enough to read a response. This was because my ```_readReg()``` function takes a length as the number of registers to read, and I was passing it directly to ```TxRx()``` which takes a length as the number of bytes to send/receive. So it was an off-by-one error. Also using the analyzer, I saw that the CC pin was never raised again after the transmission finished, despite having created a handler for this. It turned out that I never passed the function as a callback--this was easily fixed. I also see with the analyzer that my CC line is changing roughly 6us before and after my actual transmission (I am SPI clocking at 1MHz by the way). Currently this has not been addressed, but might cause timeline-related issues in the future (I am not sure, but it is worth noting).