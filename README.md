# ArduinoTrafficLightsGerman
Control 3 LEDs (ideally a real traffic light) via any IR (TV) remote.


## What is this all about?
Using an Arduino + IR Receiver + old (TV) remote to control 3 LEDS --> Ideally you got a real traffic light at home ;)

## Why do we us the Ticker.h library?
If you use the standard delay function you will have the following issues:
- If you use loops, you'll be stuck in them forever e.g. let an LED blink --> Arduino will not respond to IR receiver to get out of this
- Even with smaller delays, traffic light will remain unresponsive as long as the delay is running

## To avoid these issues you can do (at least?) 3 things:
- Use Ticker.h library to "simultaneously" execute a function while we can still execute our loop function normally and change mode whenever user presses buttons on IR remote
- Wait waiting times via timestamps (I'll add an explanation here)
- Use delays and use a 2nd (Arduino) device to interrupt delays via interrupts (easiest way but also unnecessarily expensive and less of a challenge than the other ways)

## I simply want to use your code, upload it and use my own IR remote to control the lights - how?
The current code will display the number of the IR pressed button via console so all you have to do is upload the code, open the console and press the buttons you want to use.  
Keep in mind that long pressing a button will return another value than short pressing it.  
We usually want the short press value as the long press value is the same for all buttons - at least in my case.  
Find the values for the buttons you need, write them down in a table and assign light modes to your remote codes in the "remoteCodeToMode" function.  
If, for an easy start, you simply want to have 2 buttons to go through all modes, consider using "modeSpecialGotoNextMode" and "modeSpecialGotoPreviousMode".

## How can I change basic 'settings' / time values of this code?
See all #define identifiers at the beginning of the code.  
This should include 99% of what you'd usually want to change.
