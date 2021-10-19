/* DISCIPLINE 
 *  
 * Design and make by Jasper Wang, sponsor by Seeed.
 * 
 * Date: 2021.10.20
 */

#include "TM1637.h"
#include <Grove_LED_Bar.h>
#define SPEAKER 5

/* LED bar setup */
Grove_LED_Bar bar(10, 9, 0, LED_BAR_10); // Clock pin, Data pin, Orientation

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

/* variables for led bar */

unsigned long preFadeBarTime = 0;
byte barLevel;

/* variables for led bar */
int BassTab[] = {1911, 1702, 1516, 1431, 1275, 1136, 1012};
/*                 1     2     3     4     5     6     7            */

void setup() {
  Serial.begin(9600);

  pinMode(SPEAKER, OUTPUT);
  digitalWrite(SPEAKER, LOW);

  tm1637.init();
  tm1637.set(BRIGHTEST);                       // BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

  bar.begin();
  pinMode(ledPin_Y, OUTPUT);
  pinMode(ledPin_B, OUTPUT);
  pinMode(buttonPin_Y, INPUT);
  pinMode(buttonPin_B, INPUT);

  tm1637.displayStr("HEL0");
  digitalWrite(ledPin_Y, HIGH);
  digitalWrite(ledPin_B, HIGH);
  bar.setLevel(2);
  sound(0);
  delay(1000);
  digitalWrite(ledPin_Y, LOW);
  digitalWrite(ledPin_B, LOW);
  bar.setLevel(5);
  sound(1);
  delay(1000);
  bar.setLevel(10);
  sound(2);
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
          sound(0);
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
          sound(4);

          /* Blue Button to confirm the time init here */
          if (startTiming == false && timeSet == false) {
            timeSet = true;
            selectTime = setTimeVal;
            countDown = selectTime;
            startTiming = true;
            barLevel = 10;
          }
          /* Blue Button to start the timer over and over again after setup */
          if (startTiming == false && timeSet == true && countFinish == true) {
            analogWrite(ledPin_B, 0);
            selectTime = setTimeVal;
            countDown = selectTime;
            startTiming = true;
            barLevel = 10;
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
    fadeBar();
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
        bar.setLevel(10);
        sound(5);
        delay(200);
        sound(5);
        delay(200);
        sound(5);
        delay(200);
        sound(5);
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

void fadeBar() {
  if (millis() - preFadeBarTime >= selectTime * 100) {
    bar.setLevel(barLevel);
    preFadeBarTime = millis();
    barLevel--;
  }
}

void sound(uint8_t note_index) {

  for (int i = 0; i < 40; i++)
  {
    digitalWrite(SPEAKER, HIGH);
    delayMicroseconds(BassTab[note_index]);
    digitalWrite(SPEAKER, LOW);
    delayMicroseconds(BassTab[note_index]);
  }
}
