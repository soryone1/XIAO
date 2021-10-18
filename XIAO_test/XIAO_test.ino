#include "TM1637.h"

/* Button variables setup */
const byte ledPin_Y = 0;            // Blue led is for "START"
const byte ledPin_B = 2;            // Yellow led is for "SET"
const byte buttonPin_Y = 1;
const byte buttonPin_B = 3;

int buttonState_Y;
int lastButtonState_Y = LOW;
int buttonState_B;
int lastButtonState_B = LOW;
unsigned long lastDebunceTime_Y = 0;
unsigned long lastDebunceTime_B = 0;
unsigned long debounceDelay = 50;

/* Screen variables setup */
const byte CLK = 8;
const byte DIO = 9;
TM1637 tm1637(CLK, DIO);

/* flag variables here */
bool timeSet = false;
bool startTiming = false;
bool countFinish = false;

/* variables for led fade */
unsigned long preFadeTime;
byte brightness = 0;
byte fadeAmount = 5;

/* variables for time set */
byte setTimeVal = 30;
unsigned long preSetTime;
int selectTime;

/* variables for timmer */
unsigned long preCountTime = 0;
int countDown;

void setup() {
  Serial.begin(9600);
  tm1637.init();
  tm1637.set(BRIGHTEST);                       // BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  pinMode(ledPin_Y, OUTPUT);
  pinMode(ledPin_B, OUTPUT);
  pinMode(buttonPin_Y, INPUT);
  pinMode(buttonPin_B, INPUT);

  tm1637.displayStr("HEL0");
  digitalWrite(ledPin_Y, HIGH);
  digitalWrite(ledPin_B, HIGH);
  delay(1000);
  digitalWrite(ledPin_Y, LOW);
  digitalWrite(ledPin_B, LOW);
  delay(1000);
}

void loop() {

  /* Button detect here */
  int reading_Y = digitalRead(buttonPin_Y);
  int reading_B = digitalRead(buttonPin_B);
  if (reading_Y != lastButtonState_Y) {
    lastDebunceTime_Y = millis();
  }

  if (reading_B != lastButtonState_B) {
    lastDebunceTime_B = millis();
  }

  if ((millis() - lastDebunceTime_Y ) > debounceDelay) {
    if (reading_Y != buttonState_Y) {
      buttonState_Y = reading_Y;
      if (buttonState_Y == LOW) {
        /* Yellow Button to set the timer here */
        Serial.println("Yellow");
        if (timeSet == false) {
          setTimeVal += 30;
          if (setTimeVal > 120) {
            setTimeVal = 30;
          }
        }
      }
    }

    if ((millis() - lastDebunceTime_B ) > debounceDelay) {
      if (reading_B != buttonState_B) {
        buttonState_B = reading_B;
        if (buttonState_B == LOW) {
          Serial.println("Blue");

          /* Blue Button to confirm the time init here */
          if (startTiming == false && timeSet == false) {
            timeSet = true;
            selectTime = setTimeVal;
            countDown = selectTime;
            startTiming = true;
          }
          /* Blue Button to start the timer over and over again after setup */
          if (startTiming == false && timeSet == true && countFinish == true) {
            analogWrite(ledPin_B, 0);
            selectTime = setTimeVal;
            countDown = selectTime;
            startTiming = true;
          }
        }

      }
    }
  }

  lastButtonState_Y = reading_Y;
  lastButtonState_B = reading_B;

  /* inital setup here, the yellow button light up, the blue button fade  */
  if (timeSet == false && startTiming == false) {
    digitalWrite(ledPin_Y, HIGH);
    fade(ledPin_B);
    tm1637.displayNum(setTimeVal);
  }

  /*  after the init setup, turn off the yellow button forever  */
  if (timeSet == true && startTiming == true && countFinish == false) {
    digitalWrite(ledPin_Y, LOW);
    analogWrite(ledPin_B, 0);
  }

  /*  when timer starts, begin to count down, when count finished, back to the counter state   */
  if (startTiming == true) {
    if (millis() - preCountTime > 1000) {
      preCountTime = millis();

      if (countDown > 0) {
        countDown--;
        tm1637.displayNum(countDown);
      } else {
        startTiming = false;
        countFinish = true;
        tm1637.clearDisplay();
        tm1637.displayNum(selectTime);
        analogWrite(ledPin_B, 255);
      }
    }
  }

}

void fade(int led) {
  if (millis() - preFadeTime > 20) {
    analogWrite(led, brightness);
    brightness += fadeAmount;
    if (brightness <= 0 || brightness >= 255) {
      fadeAmount = -fadeAmount;
    }
    preFadeTime = millis();
  }
}
