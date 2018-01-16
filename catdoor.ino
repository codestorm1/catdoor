/*
   Catdoor Controller

   from: Adafruit Arduino - Lesson 16. Stepper
*/

#include <DS1307RTC.h>
#include "RTClib.h"
#include <time.h>
#include <Stepper.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>

RTC_DS1307 rtc;

#define NANO // using different pinout for a nano board vs a pro mini
//#define FORCE_TIME_UPDATE
#define board_has_RTC

#ifdef NANO
  int in1Pin = 12;
  int in2Pin = 11;
  int in3Pin = 10;
  int in4Pin = 9;
#else
  int in1Pin = 13;
  int in2Pin = 12;
  int in3Pin = 11;
  int in4Pin = 10;
#endif

int ledPin = 13;

Stepper motor(512, in1Pin, in3Pin, in2Pin, in4Pin);
int previous = 0;

#ifdef NANO
  const int upButtonPin = 3; // button to open door, disables timer mode
  const int timerButtonPin = 6; // button to turn on timer mode
  const int downButtonPin = 4; // nano down button to close door, also disables timer mode
#else
  const int upButtonPin = 3; // button to open door, disables timer mode
  const int timerButtonPin = 4; // button to turn on timer mode
  const int downButtonPin = 5; // mini pro down button
#endif

bool timerMode = true;

int stepsPerRevolution = 2048;
int stepsToOpen = stepsPerRevolution * 6.25;
int stepsPerLoop = stepsToOpen / 100;

int openDoorPosition = 0;
int closedDoorPosition = stepsToOpen;
int doorPosition = stepsToOpen; // initialize as though door is closed

bool doorInMotion = false;
bool doorDirectionOpen = false;

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}

void printDigits(int digits) {
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

// trying to lower power use by disabling stepper between opening/closing
void disableStepper() {
  doorInMotion = false;
  digitalWrite(in1Pin, LOW);
  digitalWrite(in2Pin, LOW);
  digitalWrite(in3Pin, LOW);
  digitalWrite(in4Pin, LOW);
}

void enableStepper() {
  doorInMotion = true;
  digitalWrite(in1Pin, HIGH);
  digitalWrite(in2Pin, HIGH);
  digitalWrite(in3Pin, HIGH);
  digitalWrite(in4Pin, HIGH);
}

void moveDoor() {
  if (doorDirectionOpen && (doorPosition > openDoorPosition)) {
    motor.step(stepsPerLoop);
    doorPosition += stepsPerLoop * -1;
  } else if (!doorDirectionOpen && doorPosition < closedDoorPosition) {
    motor.step(stepsPerLoop * -1);
    doorPosition += stepsPerLoop;
  } else {
    digitalClockDisplay();
    Serial.print("Door ");
    Serial.println(doorDirectionOpen ? "opened" : "closed");
    digitalWrite(ledPin, LOW);
    disableStepper();
  }
}

void startDoorMove(bool open) {
  digitalClockDisplay();
  Serial.print("Door ");
  Serial.println(open ? "opening" : "closing");
  digitalWrite(ledPin, HIGH);
  enableStepper();
  doorDirectionOpen = open;
}

void morningOpen() {
  Serial.println("Timer open start");
  if (timerMode) {
    Serial.println("Time to open door");
    startDoorMove(true);
  }
}

void eveningClose() {
  Serial.println("Timer close start");
  if (timerMode) {
    Serial.println("Time to close door");
    startDoorMove(false);
  }
}


void setup()
{
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);

  pinMode(ledPin, OUTPUT);
  pinMode(upButtonPin, INPUT);
  pinMode(downButtonPin, INPUT);

  // this line is for Leonardo's, it delays the serial interface
  // until the terminal window is opened
  while (!Serial);

  Serial.println("starting up");

  Serial.begin(57600);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

#ifdef FORCE_TIME_UPDATE
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // set clock to time of last compilation
#endif

#ifdef board_has_RTC //real time clock installed
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2017, 1, 6, 0, 0, 0));
  } else {
    Serial.println("RTC is running");
  }

  setSyncProvider(RTC.get); // the function to get the time from the RTC
  if (timeStatus() != timeSet) {
    Serial.println("Time set failed");
  }
  setTime(rtc.now().unixtime());
#else /// no real time clock, set clock to last compile time
  // manually set the time
  setTime(19, 50, 0, 1, 11, 17);
#endif

  motor.setSpeed(60);
  digitalClockDisplay();

  Serial.println("Setting alarms");

  Alarm.alarmRepeat(5, 45, 11, morningOpen);
  Alarm.alarmRepeat(6, 0, 0, morningOpen);
  Alarm.alarmRepeat(20, 0, 0, eveningClose);
  digitalWrite(ledPin, LOW);
}

bool moveOnPress(int pin, bool directionUp) {
  if (digitalRead(pin) == LOW) {
    digitalClockDisplay();
    Serial.println(directionUp ? "Opening door" : "Closing door");
    startDoorMove(directionUp);
    Alarm.delay(500); // debounce
    return true;
  }
  return false;
}

void loop()
{
  moveOnPress(upButtonPin, true);
  moveOnPress(downButtonPin, false);
  
  if (digitalRead(timerButtonPin) == LOW) {
    // switch to timer mode
    digitalClockDisplay();
    Serial.println("Door motion stopped");
    timerMode = true;
    disableStepper();
    digitalWrite(ledPin, LOW);
    Alarm.delay(500); // debounce
  }

  if (doorInMotion) {
    moveDoor();
  }
  Alarm.delay(0);
}

