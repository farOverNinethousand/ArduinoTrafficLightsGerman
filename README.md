# ArduinoTrafficLightsGerman
Control 3 LEDs (ideally a real traffic light) via any IR (TV) remote.


## What does it do?
It uses an IR receiver to receive signals of usual IR remotes.
Ideally you would want to use a remote which has a red, yellow and green button (because you know, traffic lights ;)).

## Why do we us the Ticker.h library?
If you use the standard delay function you will have the following issues:
- If you use loops, you'll be stuck in them forever
- Traffic light will remain unresponsive as long as a delay function is being executed

## To avoid these issues you can do (at least?) 2 things:
- Use Ticker.h library to "simultaneously" execute a function while we can still execute our loop function normally and change mode whenever user wishes to do so
- Wait waiting times via timestamps (I'll add an explanation here)

## I simply want to use your code, upload it and use my own remote to control the lights - how?
- The current code will display the number of the IR pressed button via console so all you have to do is upload the code, open the console and press the buttons you want to use.
Keep in mind that long pressing a button will return another value than short pressing it.
We usually want the short press value.
Find the values for the buttons you need and simply but them in the "remoteCodeToMode" function.

# THIS IS NOT YET DONE!!
