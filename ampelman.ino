#include <IRremote.h>
#include <stdlib.h>
/* Source: https://github.com/sstaub/Ticker */
#include <Ticker.h>
/* Defines how long red AND yellow stay on when switching to green. German law says red/yellow phase for 50 KM/h speedlimit is 3 seconds.
   The higher the speedlimit, the more time drivers get. Source: https://de.wikipedia.org/wiki/Ampel */
#define durationYellowRedPhase 3000
/* Delay for official yellow blink signal */
#define yellowBlinkTimeOfficial 800
/* Default blink delays for individual LEDs. */
#define redBlinkTime 500
#define yellowBlinkTime 500
#define greenBlinkTime 500
/* Min/Max count number for all blinker loops */
#define minBlinkCount 3
#define maxBlinkCount 20
/* Min/max/default waittime in between blinking / lights-running */
#define minBlinkWait 200
#define defaultBlinkWait 600
#define maxBlinkWait 5000
/* Blink delay increase/decrease step */
#define blinkWaitIncreaseDecreaseStep 100
/* Min/max waittime for AutoTrafficLightPhases */
#define minTimeBetweenAutoTrafficLightPhases 1000
#define maxTimeBetweenAutoTrafficLightPhases 25000
#define minTimeBetweenBlinkRedGreenAlternating 1000
//#define maxTimeBetweenBlinkRedGreenAlternating 4000
/* Important: Defines how fast the switch relais or our lights can be switched! */
#define hardwareMaxSwitchingSpeed 100
/* Minimum delay in random mode. */
#define randomModeMinWait 300
/* Our code will never wait longer than this value (also not in random mode)! */
//#define generalMaxWaittime 120000
#define generalMaxWaittime 120000

#define PIN_RECV 11
#define PIN_LED_RED 2
#define PIN_LED_YELLOW 3
#define PIN_LED_GREEN 4

const short modeAllOn = 1, modeAllOff = 2,
            modeToggleAll = 3, modeToggleRed = 4, modeToggleYellow = 5, modeToggleGreen = 6, modeToggleRandom = 7,
            modeBlinkAll = 8, modeBlinkRed = 9, modeBlinkYellow = 10, modeBlinkGreen = 11, modeBlinkRedUserdefined = 12, modeBlinkYellowUserdefined = 13, modeBlinkGreenUserdefined = 14, modeBlinkRedGreenAlternating = 15, modeBlinkRandom = 16,
            modeGreenToRed = 17, modeRedToGreen = 18,
            modeLightsRunningFromTop = 19, modeLightsRunningFromBottom = 20, modeRandom = 21, modeRandomOnlyOne = 22,
            modeAutoTrafficLightPhases = 23,
            modeBuzzer = 24,
            /* Important: Keep this updated for nextMode/previousMode functions to work!! */
            generalMaxModeNumber = 25,
            modeSpecialIncreaseSpeed = 300, modeSpecialDecreaseSpeed = 301,
            modeSpecialGotoPreviousMode = 302, modeSpecialGotoNextMode = 303;


IRrecv irrecv(PIN_RECV);   /* An dieser Stelle wird ein Objekt definiert, dass den Infrarotsensor an Pin 11 ausliest. */

decode_results results;  /* Dieser Befehl sorgt dafür, dass die Daten, die per Infrarot eingelesen werden unter „results“ abgespeichert werden. */
unsigned long lastRemoteValue = 0;
unsigned long lastRemoteValueExecuted = 0;
unsigned long timestampLastSwitchedRed = 0;
unsigned long timestampLastSwitchedYellow = 0;
unsigned long timestampLastSwitchedGreen = 0;
/* Usually set in a time in the future for waiting purposes. */
unsigned long waitUntilTimestamp = 0;

unsigned long currentDelay = defaultBlinkWait;

/* Value of last executed mode. Never set this < 1! */
short lastMode = 1;

/* E.g. for running lights, this will count up to 2, then repeat. */
short subMode = 0;

/* States of our different lights --> Important to set this to true on init */
bool red;
bool yellow;
bool green;

/* Especially useful for random mode. This helps so that we can run light modes while we're e.g. waiting at the same time. */
Ticker test2;

bool isOn(short pin) {
  switch (pin) {
    case PIN_LED_RED:
      return red;
    case PIN_LED_YELLOW:
      return yellow;
    case PIN_LED_GREEN:
      return green;
  }
}

long getBlinkTime(short pin) {
  switch (pin) {
    case PIN_LED_RED:
      return redBlinkTime;
    case PIN_LED_YELLOW:
      return yellowBlinkTime;
    case PIN_LED_GREEN:
      return greenBlinkTime;
  }

}

void setPinStatus(short pin, bool state) {
  switch (pin) {
    case PIN_LED_RED:
      red = state;
      break;
    case PIN_LED_YELLOW:
      yellow = state;
      break;
    case PIN_LED_GREEN:
      green = state;
      break;
  }
}

void on(short pin) {
  if (!isOn(pin)) {
    digitalWrite (pin, LOW);
    setPinStatus(pin, true);
  }
}

void off(short pin) {
  if (isOn(pin)) {
    digitalWrite (pin, HIGH);
    setPinStatus(pin, false);
  }
}

bool oneIsOn() {
  return red | yellow | green;
}

void toggle(short pin) {
  if (!isOn(pin)) {
    on(pin);
  } else {
    off(pin);
  }
}

void redOn() {
  Serial.println("Red on");
  if (!red) {
    handleHardwareSwitchingSpeedLimitations(timestampLastSwitchedRed);
    on(PIN_LED_RED);
    //digitalWrite (PIN_LED_RED, LOW);
    red = true;
    timestampLastSwitchedRed = millis();
  }
}

void yellowOn() {
  Serial.println("Yellow on");
  if (!yellow) {
    handleHardwareSwitchingSpeedLimitations(timestampLastSwitchedYellow);
    digitalWrite (PIN_LED_YELLOW, LOW);
    yellow = true;
    timestampLastSwitchedYellow = millis();
  }
}

void greenOn() {
  Serial.println("Green on");
  if (!green) {
    handleHardwareSwitchingSpeedLimitations(timestampLastSwitchedGreen);
    digitalWrite (PIN_LED_GREEN, LOW);
    green = true;
    timestampLastSwitchedGreen = millis();
  }
}

void redOff() {
  Serial.println("Red off");
  if (red) {
    handleHardwareSwitchingSpeedLimitations(timestampLastSwitchedRed);
    digitalWrite (PIN_LED_RED, HIGH);
    red = false;
    timestampLastSwitchedRed = millis();
  }
}

void yellowOff() {
  Serial.println("Yellow off");
  if (yellow) {
    handleHardwareSwitchingSpeedLimitations(timestampLastSwitchedYellow);
    digitalWrite (PIN_LED_YELLOW, HIGH);
    yellow = false;
    timestampLastSwitchedYellow = millis();
  }
}


void greenOff() {
  Serial.println("Green off");
  if (green) {
    handleHardwareSwitchingSpeedLimitations(timestampLastSwitchedGreen);
    digitalWrite (PIN_LED_GREEN, HIGH);
    green = false;
    timestampLastSwitchedGreen = millis();
  }
}

/* Functions with usage of delay function (it is impossible to control device while wait is in progress) */
/* Typical traffic lights yellow blinking */
void blinkYellowOld() {
  /* TODO: Find official yellow blinking times */
  Serial.println("Blink yellow (official)");
  /* First, turn off what we do not need */
  onlyYellow();
  delay(yellowBlinkTimeOfficial);
  yellowOff();
  delay(yellowBlinkTimeOfficial);
}

void blinkAllOld(unsigned long waittime) {
  Serial.println("Blink all");
  allOn();
  delay(waittime);
  allOff();
  delay(waittime);
}

void lightsRunningFromTopOld(unsigned long waittimeMilliseconds) {
  Serial.println("Lights running from top to bottom");
  waittimeMilliseconds = fixWaittime(waittimeMilliseconds, minBlinkWait, maxBlinkWait);
  onlyRed();
  delay(waittimeMilliseconds);
  yellowOn();
  redOff();
  delay(waittimeMilliseconds);
  greenOn();
  yellowOff();
  delay(waittimeMilliseconds);
}

void lightsRunningFromBottomOld(long waittimeMilliseconds) {
  Serial.println("Lights running from bottom to top");
  waittimeMilliseconds = fixWaittime(waittimeMilliseconds, minBlinkWait, maxBlinkWait);
  onlyGreen();
  delay(waittimeMilliseconds);
  yellowOn();
  greenOff();
  delay(waittimeMilliseconds);
  redOn();
  yellowOff();
  delay(waittimeMilliseconds);
}

/* Red & green (without yellow) similar to train signals (as I remember them). */
void blinkRedGreenAlternatingOld() {
  Serial.println("Blink red green alternating");
  onlyRed();
  delay(minTimeBetweenBlinkRedGreenAlternating);
  redOff();
  greenOn();
  delay(minTimeBetweenBlinkRedGreenAlternating);
  greenOff();
}

/* Makes sure that number we use as blink count is valid. */
int fixBlinkmaxCountOld(int givenMaxCount, int fallbackMaxCount) {
  int maxCount;
  if (givenMaxCount <= 0) {
    maxCount = fallbackMaxCount;
  } else {
    maxCount = givenMaxCount;
  }
  return maxCount;
}

void greenToRedOld() {
  Serial.println("Green to red");
  /* Turn of what we don't need, leaving green on if it was already turned on before. */
  if (red || yellow) {
    allOff();
  }
  greenOn();
  delay(800);
  greenOff();
  yellowOn();
  delay(durationYellowRedPhase);
  yellowOff();
  greenOff();
  redOn();
}

void redToGreenOld() {
  Serial.println("Red to green");
  /* Turn of what we don't need, leaving red on if it was already turned on before. */
  if (yellow || green) {
    allOff();
  }
  redOn();
  delay(800);
  yellowOn();
  delay(2000);
  redOff();
  yellowOff();
  greenOn();
}
/* Functions with usage of delay function (it is impossible to control device while wait is in progress)  END*/

/* TODO: This does not yet work as expected */
void handleHardwareSwitchingSpeedLimitations(unsigned long timestampLastSwitched) {
  if (timestampLastSwitched > 0) {
    /* The timestamp at which switching is allowed */
    unsigned long timestampSwitchAllowed = timestampLastSwitched + hardwareMaxSwitchingSpeed;
    /* That timestamp minus current timestamp so if it is higher than that (switching allowed in the future), the result is the time we have to wait. */
    unsigned long waittimeUntilSwitchAllowed =  timestampSwitchAllowed - millis();
    if (waittimeUntilSwitchAllowed > 0 && waittimeUntilSwitchAllowed <= hardwareMaxSwitchingSpeed) {
      //Serial.println("You tried to switch too fast, waiting (milliseconds):");
      //Serial.println(waittimeUntilSwitchAllowed);
      delay(waittimeUntilSwitchAllowed);
    }
  }
}


void allOn() {
  Serial.println("All On");
  redOn();
  yellowOn();
  greenOn();
}

void allOff() {
  Serial.println("All off");
  redOff();
  yellowOff();
  greenOff();
}

/* If all off --> Turn all on; if ANY on turn all off (similar to most other IR devices). */
void toggleAll() {
  Serial.println("All On");
  if (!red && !yellow && !green) {
    allOn();
  } else {
    allOff();
  }
}

/* If on; turn off, if off; turn on */
void toggleRed() {
  if (!red) {
    redOn();
  } else {
    redOff();
  }
}

/* If on; turn off, if off; turn on */
void toggleYellow() {
  if (!yellow) {
    yellowOn();
  } else {
    yellowOff();
  }
}

/* If on; turn off, if off; turn on */
void toggleGreen() {
  if (!green) {
    greenOn();
  } else {
    greenOff();
  }
}

void onlyRed() {
  if (yellow) {
    yellowOff();
  }
  if (green) {
    greenOff();
  }
  if (!red) {
    redOn();
  }
}

void onlyYellow() {
  if (red) {
    redOff();
  }
  if (green) {
    greenOff();
  }
  if (!yellow) {
    yellowOn();
  }
}

void onlyGreen() {
  if (red) {
    redOff();
  }
  if (yellow) {
    yellowOff();
  }
  if (!green) {
    greenOn();
  }
}

/* Either switches from red to green or green to red like a real traffic light. */
void toggleRedGreenGreenRed() {
  if (red) {
    redToGreen();
  } else {
    greenToRed();
  }
}

void greenToRed() {
  /* Turn of what we don't need, leaving red on if it was already turned on before. */
  switch (subMode) {
    case 0:
      Serial.println("Green to red (start)");
      onlyGreen();
      waitTime(800);
      subMode++;
      break;
    case 1:
      greenOff();
      yellowOn();
      waitTime(durationYellowRedPhase);
      subMode++;
      break;
    case 2:
      yellowOff();
      redOn();
      waitTime(2500);
      subMode++;
      break;
    case 3:
      /* Do nothing as red light should be lit! */
      Serial.println("Green to red (finished)");
      break;
    default:
      subMode = 0;
      break;
  }
}

void redToGreen() {
  /* Turn of what we don't need, leaving red on if it was already turned on before. */
  switch (subMode) {
    case 0:
      Serial.println("Red to green (start)");
      onlyRed();
      waitTime(800);
      subMode++;
      break;
    case 1:
      yellowOn();
      waitTime(2000);
      subMode++;
      break;
    case 2:
      redOff();
      yellowOff();
      greenOn();
      waitTime(2500);
      subMode++;
      break;
    case 3:
      /* Do nothing as green light should be lit! */
      Serial.println("Red to green (finished)");
      break;
    default:
      subMode = 0;
      break;
  }
}

void blinkRedTicker(int maxCount, long waittime) {
  Serial.println("Blink red");
  test2.setCallback(toggleRed);
  test2.setRepeats(maxCount);
  test2.setInterval(waittime);
  test2.start();
  waitTime(maxCount, waittime);
}

void blinkYellowTicker(int maxCount, long waittime) {
  Serial.println("Blink yellow");
  test2.setCallback(toggleYellow);
  test2.setRepeats(maxCount);
  test2.setInterval(waittime);
  test2.start();
  waitTime(maxCount, waittime);
}

long blinkGreenTicker(int maxCount, long waittime) {
  Serial.println("Blink green");
  test2.setCallback(toggleGreen);
  test2.setRepeats(maxCount);
  test2.setInterval(waittime);
  test2.start();
  waitTime(maxCount, waittime);
}

/* TODO: Improve this (maybe, first, turn off whichever lights we do not need) */
/* Blink specific lights. */
void blinkTicker(short pin, short maxCount, long waittime) {
  /* 0 = blink infinitely so we should not correct that. */
  maxCount = fixBlinkmaxCountTicker(maxCount, maxBlinkCount);
  waittime = fixWaittime(waittime, minBlinkWait, maxBlinkWait);
  switch (pin) {
    case PIN_LED_RED:
      test2.setCallback(toggleRed);
      break;
    case PIN_LED_YELLOW:
      test2.setCallback(toggleYellow);
      break;
    case PIN_LED_GREEN:
      test2.setCallback(toggleGreen);
      break;
  }
  test2.setRepeats(maxCount);
  test2.setInterval(waittime);
  test2.start();
  waitTime(maxCount, waittime);
}

long blinkAllTicker(short maxCount, long waittime) {
  maxCount = fixBlinkmaxCountTicker(maxCount, maxBlinkCount);
  waittime = fixWaittime(waittime, minBlinkWait, maxBlinkWait);
  Serial.println("Blink all");
  allOff();
  test2.setCallback(toggleAll);
  test2.setRepeats(maxCount);
  test2.setInterval(waittime);
  test2.start();
  waitTime(maxCount, waittime);
  return waittime;
}

/* Toggles a random color */
void toggleRandom() {
  Serial.println("Toggle random");
  short randomNumber = getRandomPin();
  switch (randomNumber) {
    case 2:
      toggleRed();
      break;
    case 3:
      toggleYellow();
      break;
    case 4:
      toggleGreen();
      break;
  }
}

/* Turns on only one random color */
void randomOnlyOneOn() {
  Serial.println("Random one on");
  short randomNumber = getRandomPin();
  switch (randomNumber) {
    case 2:
      onlyRed();
      break;
    case 3:
      onlyYellow();
      break;
    case 4:
      onlyGreen();
      break;
  }
}

/* Turns on only one random color */
void randomMultipleOn() {
  Serial.println("Random multiple on");
  int maxNumberofLights = random(1, 3);
  for (int i = 0; i <= maxNumberofLights; i++) {
    short randomNumber = getRandomPin();
    switch (randomNumber) {
      case 2:
        redOn();
        break;
      case 3:
        yellowOn();
        break;
      case 4:
        greenOn();
        break;
    }
  }
}

/* Makes sure that waittime we use for blinking purposes is valid. */
int fixWaittime(long givenWaitMilliseconds, long minMilliseconds, long maxMilliseconds) {
  int waitMilliseconds;
  if (givenWaitMilliseconds < minMilliseconds) {
    /* Waittime too low --> Fallback to lowest value */
    waitMilliseconds = minMilliseconds;
  } else if (givenWaitMilliseconds > maxMilliseconds) {
    /* Waittime too high --> Fallback to max value */
    waitMilliseconds = maxMilliseconds;
  } else {
    /* Waittime okay --> Use it */
    waitMilliseconds = givenWaitMilliseconds;
  }
  return waitMilliseconds;
}

/* Makes sure that number we use as blink count is valid. */
int fixBlinkmaxCountTicker(int givenMaxCount, int fallbackMaxCount) {
  int maxCount;
  /* 0 = endless e.g. endless blinking */
  if (givenMaxCount < 0 || givenMaxCount > fallbackMaxCount) {
    maxCount = fallbackMaxCount;
  } else {
    maxCount = givenMaxCount;
  }
  return maxCount;
}

/* TODO: Yellow phase is not yet nice here */
void autoTrafficLightPhases() {
  /* TODO: Improve this as it may change to other modes while current mode is not finished (e.g. in between blinking). */
  int randomNumber = rand() % 2;
  if ((subMode == 0 && randomNumber == 1) || subMode > 0) {
    toggleRedGreenGreenRed();
    if (subMode == 3) {
      /* TODO: Maybe find a more elegant way to do this */
      subMode = 0;
    }
  } else {
    /* Reset subMode for phase change */
    subMode = 0;
    onlyYellow();
    blinkTicker(PIN_LED_YELLOW, 6, yellowBlinkTimeOfficial);
  }
}

/* TODO: Define, how many phases we want to have */
void autoTrafficLightPhasesTicker() {
  Serial.println("Auto traffic light phases");
  int waittime = random(minTimeBetweenAutoTrafficLightPhases, maxTimeBetweenAutoTrafficLightPhases);
  test2.setCallback(autoTrafficLightPhases);
  test2.setInterval(waittime);
  test2.start();
}

/* Execute this 3 times for one run. */
void lightsRunningFromTopTicker() {
  switch (subMode) {
    case 0:
      onlyRed();
      subMode++;
      break;
    case 1:
      onlyYellow();
      subMode++;
      break;
    case 2:
      onlyGreen();
      subMode = 0;
      break;
    default:
      /* This should never happen! */
      subMode = 0;
      break;
  }
}

/* Run this either in infinite mode (maxCount = 0) or1 time --> maxCount = 3 (each call = 1 stage!) */
void lightsRunningFromTop(short maxCount, unsigned long waittime) {
  maxCount = fixBlinkmaxCountTicker(maxCount, maxBlinkCount);
  test2.setCallback(lightsRunningFromTopTicker);
  test2.setRepeats(maxCount);
  test2.setInterval(waittime);
  test2.start();
  waitTime(maxCount, waittime);
}

/* Execute this 3 times for one run. */
void lightsRunningFromBottomTicker() {
  Serial.println("Lights running from top to bottom");
  switch (subMode) {
    case 0:
      onlyGreen();
      subMode++;
      break;
    case 1:
      onlyYellow();
      subMode++;
      break;
    case 2:
      onlyRed();
      subMode = 0;
      break;
    default:
      /* This should never happen! */
      subMode = 0;
      break;
  }
}

/* Run this either in infinite mode (maxCount = 0) or1 time --> maxCount = 3 (each call = 1 stage!) */
void lightsRunningFromBottom(short maxCount, unsigned long waittime) {
  test2.setCallback(lightsRunningFromBottomTicker);
  test2.setRepeats(maxCount);
  test2.setInterval(waittime);
  test2.start();
  waitTime(maxCount, waittime);
}

void blinkRandom(short maxCount, unsigned long waittime) {
  blinkTicker(getRandomPin(), maxCount, waittime);
}

/* Red & green (without yellow) similar to train signals (as I remember them). */
void blinkRedGreenAlternatingTicker() {
  switch (subMode) {
    case 0:
      if (yellow) {
        yellowOff();
      }
      greenOff();
      redOn();
      subMode++;
      break;
    case 1:
      redOff();
      greenOn();
      subMode = 0;
      break;
    default:
      /* This should never happen! */
      subMode = 0;
      break;
  }
}

/* Red & green (without yellow) similar to train signals (as I remember them). */
void blinkRedGreenAlternating(short maxCount, unsigned long waittime) {
  Serial.println("Blink red green alternating");
  test2.setCallback(blinkRedGreenAlternatingTicker);
  test2.setRepeats(maxCount);
  test2.setInterval(waittime);
  test2.start();
  waitTime(maxCount, waittime);
}

void blinkShowUserError() {
  //allOff();
  blinkTicker(PIN_LED_RED, 6, hardwareMaxSwitchingSpeed);
}

void randomBlinkMode() {
  /* TODO: Add use this somewhere */
  Serial.println("Random blink mode");
  int randomNumberForMode = random(0, 5);
  int randomNumberForNumberofBlinks = getRandomNumberOfBlinks();
  long randomNumberForBlinkDelay = getRandomDelayBetweenBlinks();
  switch (randomNumberForMode) {
    case 0:
      blinkAllTicker(randomNumberForNumberofBlinks, randomNumberForBlinkDelay);
      break;
    case 1:
      blinkRedGreenAlternating(randomNumberForNumberofBlinks, randomNumberForBlinkDelay);
      break;
    case 2:
      blinkTicker(PIN_LED_RED, randomNumberForNumberofBlinks, randomNumberForBlinkDelay);
      break;
    case 3:
      blinkTicker(PIN_LED_YELLOW, randomNumberForNumberofBlinks, randomNumberForBlinkDelay);
      break;
    case 4:
      blinkTicker(PIN_LED_GREEN, randomNumberForNumberofBlinks, randomNumberForBlinkDelay);
      break;
    default:
      Serial.println("This should never happen");
      blinkTicker(PIN_LED_RED, randomNumberForNumberofBlinks, randomNumberForBlinkDelay);
      break;
  }
}

/* Activates a random mode. */
void randomLightMode() {
  /* TODO: Improve this as it may change to other modes while current mode is not finished (e.g. in between blinking). */
  /* TODO: This may not wait long enough for our light mode to get fully executed --> Either just make it wait longer or add functionality to find out how long exactly a function needs to be fully executed! */
  Serial.println("Random light mode");
  /* First make sure everything is turned off */
  allOff();
  /* TODO: Why isn't this really random?? */
  int randomNumberForMode = random(0, 12);
  Serial.println("RandomMode:");
  Serial.println(randomNumberForMode);
  int randomNumberForNumberofBlinks = getRandomNumberOfBlinks();
  long randomNumberForBlinkDelay = getRandomDelayBetweenBlinks();
  long randomNumberForDelay = getRandomWaitTime();
  switch (randomNumberForMode) {
    case 0:
      redOn();
      break;
    case 1:
      yellowOn();
      break;
    case 2:
      greenOn();
      break;
    case 3:
      greenToRed();
      break;
    case 4:
      redToGreen();
      break;
    case 5:
      allOn();
      break;
    case 6:
      blinkTicker(PIN_LED_RED, randomNumberForNumberofBlinks, randomNumberForBlinkDelay);
      break;
    case 7:
      blinkTicker(PIN_LED_YELLOW, randomNumberForNumberofBlinks, randomNumberForBlinkDelay);
      break;
    case 8:
      blinkTicker(PIN_LED_GREEN, randomNumberForNumberofBlinks, randomNumberForBlinkDelay);
      break;
    case 9:
      blinkRandom(randomNumberForNumberofBlinks, randomNumberForBlinkDelay);
      break;
    case 10:
    /* TODO: Improve this */
      lightsRunningFromTop(100, currentDelay);
      break;
    case 11:
    /* TODO: Improve this */
      lightsRunningFromBottom(100, currentDelay);
      break;
    default:
      /* Do nothing - only wait if required */
      break;
  }
  waitTime(randomNumberForDelay);
}

int getRandomNumberOfBlinks() {
  return random(minBlinkCount, maxBlinkCount);
}

int getRandomDelayBetweenBlinks() {
  return random(minBlinkWait, maxBlinkWait + 1);
}

long getRandomWaitTime() {
  return random(randomModeMinWait, generalMaxWaittime + 1);
}

short getNextMode() {
  short nextMode = lastMode + 1;
  if (nextMode > generalMaxModeNumber) {
    blinkShowUserError();
    return lastMode;
  } else {
    return nextMode;
  }
}

short getPreviousMode() {
  short nextMode = lastMode - 1;
  if (nextMode < 1) {
    blinkShowUserError();
    return lastMode;
  } else {
    return nextMode;
  }
}

/* Decreases waittime for blinking */
/* TODO: Maybe use another effect for showing current speed */
void increaseBlinkSpeed() {
  Serial.println("Increasing blink speed");
  long tempDelay = currentDelay - blinkWaitIncreaseDecreaseStep;
  if (tempDelay < minBlinkWait) {
    Serial.println("Failed to increase blink speed: New delay value is too low:");
    Serial.println(tempDelay);
    /* New value is too low --> Show error */
    blinkShowUserError();
  } else {
    Serial.println("delay has changed");
    Serial.println("delay_old:");
    Serial.println(currentDelay);
    /* Set new blink-speed */
    currentDelay = tempDelay;
    /* Demonstrate new blink-speed */
    lightsRunningFromBottom(3, currentDelay);
  }
  Serial.println("currentDelay:");
  Serial.println(currentDelay);
}

/* Increases waittime for blinking  */
/* TODO: Maybe use another effect for showing current speed */
void decreaseBlinkSpeed() {
  Serial.println("Decreasing blink speed");
  long tempDelay = currentDelay + blinkWaitIncreaseDecreaseStep;
  if (tempDelay > maxBlinkWait) {
    Serial.println("Failed to decrease blink speed: New delay value is too high:");
    Serial.println(tempDelay);
    /* New value is too high --> Show error */
    blinkShowUserError();
  } else {
    Serial.println("delay has changed:");
    Serial.println("delay_old:");
    Serial.println(currentDelay);
    /* Set new blink-speed */
    currentDelay = tempDelay;
    /* Demonstrate new blink-speed */
    lightsRunningFromBottom(3, currentDelay);
  }
  Serial.println("currentDelay:");
  Serial.println(currentDelay);
}

/* Drinking game mode :) */
void modeBuzzr(bool buttonPressed) {
  if (buttonPressed) {
    allOff();
    waitTime(random(1500, 3501));
  } else if (!oneIsOn()) {
    randomMultipleOn();
    //randomOnlyOneOn();
  }

}

/* Put init 'animation' here */
void showStartSequence() {
  /* Start sequence */
  lightsRunningFromBottomOld(150);
  redOff();
  delay(200);
  lightsRunningFromTopOld(150);
  delay(200);
  blinkAllOld(200);
  delay(200);
  allOn();
}

/* Returns random number 2, 3 or 4.  */
short getRandomPin() {
  long randomNumber = random(2, 5);
  return (short) randomNumber;
}

void waitTime(short count, unsigned long waittime) {
  unsigned long final_waittime;
  if (count == 0) {
    /* Infinite Ticker loop --> wait waittime once */
    final_waittime = waittime;
  } else {
    final_waittime = count * waittime;
  }
  /* TODO: Check this - enabling this again might fix random mode! */
  //waitTime(final_waittime);
}

/* Handles waitime */
void waitTime(unsigned long waittime) {
  if (waitUntilTimestamp == 0 || waitUntilTimestamp < millis()) {
    waitUntilTimestamp = millis() + waittime;
  } else {
    waitUntilTimestamp += waittime;
  }
}

/* Unused */
void stopAllTickers() {
  test2.stop();
}

bool isWaiting() {
  bool waiting = millis() < waitUntilTimestamp;
  if (waiting) {
    /* Just make sure that we do not have an abnormally high waittime! */
    unsigned long waittimeLeft = waitUntilTimestamp - millis();
    Serial.println("WaittimeLeft:");
    Serial.println(waittimeLeft);
    if (waittimeLeft > generalMaxWaittime) {
      waitUntilTimestamp = 0;
      waiting = false;
    }
  }
  return waiting;
}

/* Converts IR received codes to number to be used in launchMode */
short remoteCodeToMode(unsigned long remoteValue) {
  switch (remoteValue) {
    case 4278227565:
      /* Button power */
      return modeToggleAll;
    case 4278243885:
      /* Button mute */
      return modeAllOff;
    case 4278225015:
      /* Button red */
      return modeToggleRed;
    case 4278241335:
      /* Button yellow */
      return modeToggleYellow;
    case 4278208695:
      /* Button green */
      return modeToggleGreen;
    case 4278192885:
      /* Button blau */
      return modeToggleRandom;
    case 4278254085:
      return modeRandom;
    case 4278221445:
      /* TODO: Maybe add functionality to 'fill uf' light from bottom to top (green to red) */
      /* Button CH up */
      return modeGreenToRed;
    case 4278245415:
      /* TODO: Maybe add functionality to 'fill uf' light from btop to bottom (red to green) */
      /* Button CH down */
      return modeRedToGreen;
    case 4278220935:
      /* Button left */
      return modeLightsRunningFromTop;
    case 4278205125:
      /* Button right */
      return modeLightsRunningFromBottom;
    case 4278237765:
      /* Button SUB-T */
      return modeBlinkAll;
    case 4278253575:
      /* Button OK */
      return modeBlinkRedGreenAlternating;
    case 4278233175:
      /* Audio --> Is located above red button */
      return modeBlinkRed;
    case 4278249495:
      /* Button EPG --> Is located above yellow button */
      return modeBlinkYellow;
    case 4278216855:
      /* Button mode --> Is located above green button */
      return modeBlinkGreen;
    case 4278213285:
      /* Button 0 */
      /* TODO: Put something else here! */
      return modeBlinkRandom;
    case 4278235725:
      /* Button 1 */
      return modeAutoTrafficLightPhases;
    case 4278219405:
      /* Button 2 */
      return modeLightsRunningFromTop;
    case 4278252045:
      /* Button 3 */
      return modeLightsRunningFromBottom;
    case 4278225525:
      /* Button 4 */
      return modeRandomOnlyOne;
    case 4278209205:
      return modeBuzzer;
    case 4278201045:
      /* Button FAVOR */
      return modeBlinkRandom;
    case 4278212775:
      /* Button PG- */
      return modeSpecialDecreaseSpeed;
    case 4278196965:
      /* Button PG+ */
      return modeSpecialIncreaseSpeed;
    case 4278235215:
      /* Button INFO */
      return modeBlinkRedUserdefined;
    case 4278218895:
      /* Button SLEEP */
      return modeBlinkYellowUserdefined;
    case 4278251535:
      /* Button SUB-T */
      return modeBlinkGreenUserdefined;
    case 4278203085:
      /* Button TEXT */
      /* TODO: Put something else here! */
      return modeBlinkRandom;
    case 4278245925:
      /* Button LAST */
      return modeSpecialGotoPreviousMode;
    case 4278229605:
      /* Button TV/R */
      return modeSpecialGotoNextMode;
    case 16714740:
      /* Sky Remote Ok Button */
      return modeBuzzer;
    case 16742790:
      /* Sky remote Power */
      return modeToggleAll;
    case 16734630:
      /* Sky remote Help */
      return modeBlinkAll;
    default:
      /* Buttom long press / unhandled IR code */
      return 0;
  }
  /* TODO: Fixme Maybe change this to 'lastRemoteValueReceived' */
  lastRemoteValueExecuted = remoteValue;
}


void launchMode(short mode, bool userPressedButton) {
  if (userPressedButton) {
    Serial.println("User pressed button");
  }
  /* Only save number of last mode if  it was NOT a special mode! */
  bool saveCurrentModeNumber = false;
  bool currentModeIsSpecialMode = isSpecialMode(mode);
  /* First handle specialModes */
  if (currentModeIsSpecialMode) {
    short modeNew = mode;
    switch (mode) {
      /* TODO: Maybe add loggers for user-mode changes */
      case modeSpecialIncreaseSpeed:
        if (userPressedButton) {
          increaseBlinkSpeed();
        }
        break;
      case modeSpecialDecreaseSpeed:
        if (userPressedButton) {
          decreaseBlinkSpeed();
        }
        break;
      case modeSpecialGotoPreviousMode:
        if (userPressedButton) {
          modeNew = getPreviousMode();
          /* User has changed mode number --> We can save current number */
          saveCurrentModeNumber = true;
        }
        break;
      case modeSpecialGotoNextMode:
        if (userPressedButton) {
          modeNew = getNextMode();
          saveCurrentModeNumber = true;
        }
        break;
      default:
        saveCurrentModeNumber = true;
        break;
    }
    /* Check if user has changed mode via button press e.g. next/previous mode. */
    if (modeNew != mode) {
      Serial.println("Mode has been changed via increase/decrease mode");
      /* E.g. all lights are off and current mode is 1 (= toggle all) - lights would just stay off if they were off in the first place when we reached this mode.
         This is why we simply pretend that the user has pressed this mode button.
      */
      userPressedButton = true;
      mode = modeNew;
    }
  } else {
    saveCurrentModeNumber = true;
  }
  /* Check if mode has changed compared to the last real mode. Ignores special modes. */
  if (mode != 0 && !currentModeIsSpecialMode && mode != lastMode) {
    Serial.println("Mode has changed compared to last mode");
    Serial.println("mode_old:");
    Serial.println(lastMode);
    Serial.println("mode_new:");
    Serial.println(mode);
    /* Reset all. */
    /* Stop 'parallel' running lightmodes. If we don't do this, we may run into total chaos!*/
    Serial.println("Stopping all tickers & resetting subMode");
    /* Stop 'parallel' running function calls / blinkers */
    stopAllTickers();
    /* Reset subMode so that functions like lightsRunning will start/stop correctly */
    subMode = 0;
    /* Reset eventually existing waittimes as user has performed action. */
    waitUntilTimestamp = 0;
  } else {
    //Serial.println("mode_current:");
    //Serial.println(mode);
  }

  /* Now handle normalModes */
  switch (mode) {
    case modeAllOn:
      if (userPressedButton) {
        allOn();
      }
      break;
    case modeAllOff:
      if (userPressedButton) {
        allOff();
      }
      break;
    case modeToggleAll:
      if (userPressedButton) {
        toggleAll();
      }
      break;
    case modeToggleRed:
      if (userPressedButton) {
        toggleRed();
      }
      break;
    case modeToggleYellow:
      if (userPressedButton) {
        toggleYellow();
      }
      break;
    case modeToggleGreen:
      if (userPressedButton) {
        toggleGreen();
      }
      break;
    case modeRandom:
      randomLightMode();
      break;
    case modeGreenToRed:
      /* TODO: Maybe add functionality to 'fill up' light from bottom to top (green to red) */
      greenToRed();
      break;
    case modeRedToGreen:
      /* TODO: Maybe add functionality to 'fill up' light from btop to bottom (red to green) */
      redToGreen();
      break;
    case modeLightsRunningFromTop:
      lightsRunningFromTop(100, currentDelay);
      break;
    case modeLightsRunningFromBottom:
      lightsRunningFromBottom(100, currentDelay);
      break;
    case modeBlinkAll:
      blinkAllTicker(0, 1000);
      break;
    case modeBlinkRedGreenAlternating:
      blinkRedGreenAlternating(0, 1000);
      break;
    case modeBlinkRed:
      blinkTicker(PIN_LED_RED, 0, 1000);
      break;
    case modeBlinkYellow:
      blinkTicker(PIN_LED_YELLOW, 0, 1000);
      break;
    case modeBlinkGreen:
      blinkTicker(PIN_LED_GREEN, 0, 1000);
      break;
    case modeToggleRandom:
      if (userPressedButton) {
        toggleRandom();
      }
      break;
    case modeAutoTrafficLightPhases:
      autoTrafficLightPhases();
      break;
    case modeBlinkRedUserdefined:
      blinkTicker(PIN_LED_RED, 0, currentDelay);
      break;
    case modeBlinkYellowUserdefined:
      blinkTicker(PIN_LED_YELLOW, 0, currentDelay);
      break;
    case modeBlinkGreenUserdefined:
      blinkTicker(PIN_LED_GREEN, 0, currentDelay);
      break;
    case modeBlinkRandom:
      /* TODO: Fix this mode */
      blinkRandom(0, currentDelay);
      break;
    case modeBuzzer:
      modeBuzzr(userPressedButton);
      break;
    case modeRandomOnlyOne:
      if (userPressedButton) {
        randomOnlyOneOn();
      }
      break;
    default:
      break;
  }
  /* Ignore invalid modes and "settings" modes. */
  if (mode > 0 && saveCurrentModeNumber) {
    /* Remember last executed mode */
    lastMode = mode;
  }
}

void setup()
{

  Serial.begin(9600);    /*Im Setup wird die Serielle Verbindung gestartet, damit man sich die Empfangenen Daten der Fernbedienung per seriellen Monitor ansehen kann. */

  Serial.println("Setting up");

  pinMode (PIN_LED_RED, OUTPUT);
  pinMode (PIN_LED_YELLOW, OUTPUT);
  pinMode (PIN_LED_GREEN, OUTPUT);


  irrecv.enableIRIn();   /* Dieser Befehl initialisiert den Infrarotempfänger. */

  /* Important: All lights are on when the microproessor starts! If you set some to false, you may get strange light constellations until every light is switched on/off once via code. */
  //lightsRunningFromBottom(4, minBlinkWait);
  red = true;
  yellow = true;
  green = true;
  allOff();
  showStartSequence();
  delay(2000);
  /* Set start-mode */
  lastMode = modeRandom;
}

bool tickerIsRunning() {
  /* Ticker will stay in RUNNING state although repeats are exceeded --> This function is a small wrapper to get the 'real' state of our ticker (getRepeats=0 --> infinite run --> We return false here as we cannot wait for an infinite Ticker to finish ;) ). */
  return test2.getState() == RUNNING && (test2.getRepeats() == 0 || test2.getRepeatsCounter() < test2.getRepeats());
}

/* Special modes are modes which are no light modes themselves but change settings such as blink delays or go to next/previous mode. */
bool isSpecialMode(short mode) {
  return mode >= 300;
}

/* Der loop-Teil fällt durch den Rückgriff auf die „library“ sehr kurz aus. */
void loop() {
  /* Ticker aktiv halten */
  test2.update();

  if (irrecv.decode(&results)) {    /* Wenn Daten empfangen wurden, */
    Serial.println(results.value, DEC); /* werden sie als Dezimalzahl (DEC) an den Serial-Monitor ausgegeben. */

    unsigned long currentRemoteValue = results.value;
    short currentMode = remoteCodeToMode(currentRemoteValue);
    /* Only launch mode if user has pressed a known button! */
    if (currentMode != 0) {
      launchMode(currentMode, true);
      lastRemoteValue = currentRemoteValue;
    }
    /* Der nächste Wert soll vom IR-Empfänger eingelesen werden */
    irrecv.resume();
  } else if (!isWaiting() && !tickerIsRunning()) {
    /* Only enter this if we're not currently waiting! */
    launchMode(lastMode, false);
  }

}
