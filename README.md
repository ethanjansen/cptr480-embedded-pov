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
   > In order to have as much resolution as possible I want to fill (or close to it) the ```MOD ```register, but I will change the ```prescaleDivisor``` as slow as possible while still not seeing any flicker. At about 92Hz the flicker is gone unless I move it around, then I might be able to see it. At about 184Hz it is completly gone. I will aim for 184Hz with a ```prescaleDivisor``` of 4.
 - Question 5 Answer:
   > I found that a PIT interrupt frequency from 10-20Hz appears to cause smooth fading. I will choose 17Hz (it is quite smooth, but doesn't cycle too fast).
 - Final Implementation Notes:
   > Originally I had created a TPM driver where I could change the ```CnV``` value by specifying a duty cycle "percentage" from 0-10000 (0-100.00%). While this mostly worked with an exponential fade, it did not work well with the squaring implementation of gamma correction. In order to address this I added a way of setting ```CnV``` directly with the TPM driver. Also I decided to keep the RED LED pulsing effect as an option, but I (for some reason) preferred the exponential effect rather than the (seemingly) linear transition--the exponential effect was too fast for color cycling. Because of this my ```LED``` class supports both exponential pulsing and linear cycling.
