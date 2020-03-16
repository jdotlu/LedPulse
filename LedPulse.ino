/*
   Pulses an LED light strip based on serial input.

   Input must be 13 characters long with a newline at the end.
   It must be in the following format:
     char 1-3: red value (use "rrr" for random value of red, not used for hold pulse type)
     char 4-6: blue value (use "rrr" for random value of blue, not used for hold pulse type)
     char 7-9: green value (use "rrr" for random value of green, not used for hold pulse type)
     char 10-12: bpm value (not used for hold pulse type)
     char 13: pulse type (s=strobe, f=fade, h=hold)

   Example input is: 255255255120s, which will strobe white for 120bpm.
*/

#define RED_LED 6
#define BLUE_LED 3
#define GREEN_LED 5

// the bigger the value, the smoother fade transitions will appear
const double FADE_STEPS = 100;
// for 4/4 beats
const int BEATS_PER_BAR = 4;
// how long to delay each time before monitoring serial input
const unsigned long DELAY_STEP_DURATION_MS = 10;

unsigned long delayMs = 500;  //start at 120bpm
unsigned int rVal = 255;
unsigned int gVal = 255;
unsigned int bVal = 255;
char pulseType = 'h';
boolean randomRed = false;
boolean randomGreen = false;
boolean randomBlue = false;

void setup() {
  Serial.begin(9600);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
}


void loop() {
  if (Serial.available() > 0) {
    readInput();
  }

  switch (pulseType) {
    case 'h':
      hold();
      break;
    case 's':
      strobe();
      break;
    case 'f':
      fade();
      break;
  }
}

void readInput() {
  String input = Serial.readStringUntil('\n');

  if (input.length() != 13) {
    Serial.println("Bad input");
  } else {
    String redInput = input.substring(0, 3);
    if (redInput == "rrr") {
      rVal = 255;
      randomRed = true;
    } else {
      rVal = redInput.toInt();
      randomRed = false;
    }

    String greenInput = input.substring(3, 6);
    if (greenInput == "rrr") {
      gVal = 255;
      randomGreen = true;
    } else {
      gVal = greenInput.toInt();
      randomGreen = false;
    }

    String blueInput = input.substring(6, 9);
    if (blueInput == "rrr") {
      bVal = 255;
      randomBlue = true;
    } else {
      bVal = blueInput.toInt();
      randomBlue = false;
    }

    int sVal = input.substring(9, 12).toInt();
    delayMs = calculateBpmDelay(sVal);

    pulseType = input.charAt(12);

    printDebug();
  }
}

void printDebug() {
  Serial.print("red=");
  Serial.println(rVal);
  Serial.print("green=");
  Serial.println(gVal);
  Serial.print("blue=");
  Serial.println(bVal);
  Serial.print("delayMs=");
  Serial.println(delayMs);
  Serial.print("pulseType=");
  Serial.println(pulseType);
}

unsigned long calculateBpmDelay(int sVal) {
  unsigned long beatLengthMs = 60000 / sVal;
  return beatLengthMs / 2;
}

void hold() {
  analogWrite(RED_LED, rVal);
  analogWrite(GREEN_LED, gVal);
  analogWrite(BLUE_LED, bVal);
}

void strobe() {
  while (true) {
    analogWrite(RED_LED, getRedValue());
    analogWrite(GREEN_LED, getGreenValue());
    analogWrite(BLUE_LED, getBlueValue());

    if (!interruptableDelay(delayMs)) {
      return;
    }

    analogWrite(RED_LED, 0);
    analogWrite(GREEN_LED, 0);
    analogWrite(BLUE_LED, 0);

    if (!interruptableDelay(delayMs)) {
      return;
    }
  }
}

void fade() {
  unsigned int thisRVal = getRedValue();
  unsigned int thisGVal = getGreenValue();
  unsigned int thisBVal = getBlueValue();

  while (true) {
    for (double i = 1; i > 0; i = i - (1 / FADE_STEPS) ) {
      analogWrite(RED_LED, thisRVal * i);
      analogWrite(GREEN_LED, thisGVal * i);
      analogWrite(BLUE_LED, thisBVal * i);

      // fade for the entire bar
      if (!interruptableDelay(delayMs / FADE_STEPS * BEATS_PER_BAR)) {
        return;
      }
    }

    thisRVal = getRedValue();
    thisGVal = getGreenValue();
    thisBVal = getBlueValue();

    for (double i = 0; i < 1; i = i + (1 / FADE_STEPS) ) {
      analogWrite(RED_LED, thisRVal * i);
      analogWrite(GREEN_LED, thisGVal * i);
      analogWrite(BLUE_LED, thisBVal * i);

      if (!interruptableDelay(delayMs / FADE_STEPS * BEATS_PER_BAR)) {
        return;
      }
    }
  }
}

/*
   Returns true if not interrupted.  Returns false if interrupted.
*/
boolean interruptableDelay(unsigned long durationMs) {

  for (unsigned long i = 0; i < durationMs; i = i + DELAY_STEP_DURATION_MS) {
    if (Serial.available() > 0) {
      return false;
    }

    if (i + DELAY_STEP_DURATION_MS > durationMs) {
      delay(durationMs - i);
      return true;
    }

    delay(DELAY_STEP_DURATION_MS);
  }

  return true;
}

unsigned int getRedValue() {
  if (randomRed) {
    return random(1, 256);
  } else {
    return rVal;
  }
}

unsigned int getGreenValue() {
  if (randomGreen) {
    return random(1, 256);
  } else {
    return gVal;
  }
}

unsigned int getBlueValue() {
  if (randomBlue) {
    return random(1, 256);
  } else {
    return bVal;
  }
}
