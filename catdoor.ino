
/*
 * Catdoor Controller 
 * 
 * from: Adafruit Arduino - Lesson 16. Stepper
*/

#include <Stepper.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

#define NANO

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

const int upButtonPin = 3; // button to open door, disables timer mode
const int timerButtonPin = 4; // button to turn on timer mode
#ifdef NANO
const int downButtonPin = 6; // nano down button to close door, also disables timer mode
#else
const int downButtonPin = 5; // mini pro down button
#endif

bool timerMode = true;

int stepsPerRevolution = 2048;
int stepsToOpen = stepsPerRevolution * 6.25;
int stepsPerLoop = stepsToOpen / 100;

int openDoorPosition = 0;
int closedDoorPosition = stepsToOpen;

//int doorPosition = 0; // initialize as though door is up
int doorPosition = stepsToOpen; // initialize as though door is down

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

void moveDoor() {
  Serial.println("in Move door");
    Serial.println(doorDirectionOpen);
    Serial.println(doorPosition);
    Serial.println(openDoorPosition);
    Serial.println(closedDoorPosition);
  if (doorDirectionOpen && (doorPosition > openDoorPosition)) {
    Serial.println("Move door open");
    motor.step(stepsPerLoop);
    doorPosition += stepsPerLoop * -1;
  } else if (!doorDirectionOpen && doorPosition < closedDoorPosition) {
    Serial.println("Move door closed");
    motor.step(stepsPerLoop * -1);
    doorPosition += stepsPerLoop;
  } else {
      doorInMotion = false;
      digitalWrite(ledPin, LOW);
  }
}

void startDoorMove(bool open) {
  doorInMotion = true;
  digitalWrite(ledPin, HIGH);
  doorDirectionOpen = open;
}

void morningOpen() {
  Serial.println("Open sez me");
  if (timerMode) {
    Serial.println("Time to open door");
    startDoorMove(true);
  }
}

void eveningClose() {
  Serial.println("Close sez me");
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
// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
  } else {
    Serial.println("RTC is running");
    DateTime now = rtc.now();
    setTime(now.unixtime());
  }
  
  motor.setSpeed(60);
  digitalClockDisplay();
//setTime(1,53,0,1,1,17);
//setTime(0,0,0,1,1,17);
//  setTime(21,5,0,1,1,17);
  Serial.println("Setting alarms");
  Alarm.alarmRepeat(7, 0, 0, morningOpen);  // 7am every day
  Alarm.alarmRepeat(20, 0, 0, eveningClose );  // 8pm every night
 //rtc.adjust(DateTime(2017, 1, 6, 0, 0, 0));
//  Alarm.alarmRepeat(0, 0, 20, morningOpen);
//  Alarm.alarmRepeat(0, 1, 0, eveningClose);
//  Alarm.timerRepeat(5, eveningClose);
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
  if (moveOnPress(upButtonPin, true)) {
    return;
  }
  
  if (moveOnPress(downButtonPin, false)) {
    return;
  }
  
  if (digitalRead(timerButtonPin) == LOW) {
    // switch to timer mode
    digitalClockDisplay();
    Serial.println("Timer mode enabled");
    timerMode = true;
    doorInMotion = false;
    digitalWrite(ledPin, LOW);
    Alarm.delay(500); // debounce
    return;
  }

  if (doorInMotion) {
    Serial.println("Moving door a bit");
    moveDoor();
  }
  //Serial.println(now());
  Alarm.delay(0);
}

