# CPTR480 Labs & Projects

## Lab 01:
 - Question 2 Answer:
   > The PIT is still running based on the peripheral register view. After getting to the end of the handler via step over/step into the debugger loses all information (about variables, the stack, registers, etc.) but the process is still paused somewhere outside of what is visible. I watched that ```g_pitTicks``` is incremented (as expected) after every continue. When watching a random ```int``` that I added to main it behaved as expected but I was unable to watch it while I was in some other scope--such as when inside the PIT interrupt handler.
 - Question 4 Answer:
   > 
