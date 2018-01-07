
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

int in1Pin = 13;
int in2Pin = 12;
int in3Pin = 11;
int in4Pin = 10;
int ledPin = 13;

Stepper motor(512, in1Pin, in3Pin, in2Pin, in4Pin);  
int previous = 0;

const int upButtonPin = 3; // button to open door, disables timer mode
const int timerButtonPin = 4; // button to turn on timer mode
const int downButtonPin = 5; // button to close door, also disables timer mode

bool timerMode = true;

int stepsPerRevolution = 2048;
int stepsToOpen = stepsPerRevolution * 6.25;
int stepsPerLoop = stepsToOpen / 100;

int openDoorPosition = 0;
int closedDoorPosition = stepsToOpen;

int doorPosition = 0; // initialize as though door is down

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
//    if (doorPosition <= openDoorPosition) {
//      doorInMotion = false;
//      digitalWrite(ledPin, LOW);
//    }
  } else if (!doorDirectionOpen && doorPosition < closedDoorPosition) {
    Serial.println("Move door closed");
    motor.step(stepsPerLoop * -1);
    doorPosition += stepsPerLoop;
//    if (doorPosition >= closedDoorPosition) {
//      doorInMotion = false;
//      digitalWrite(ledPin, LOW);
//    }
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

void MorningOpen() {
  Serial.println("Open sez me");
  if (timerMode) {
    Serial.println("Time to open door");
    startDoorMove(true);
  }
}

void EveningClose() {
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
    
  }
  DateTime now = rtc.now();
  setTime(now.unixtime()); 
  motor.setSpeed(60);
  digitalClockDisplay();
  
//  Alarm.alarmRepeat(7, 0, 0, MorningOpen);  // 7am every day
//  Alarm.alarmRepeat(20, 0, 0, EveningClose );  // 8pm every night
 rtc.adjust(DateTime(2017, 1, 6, 0, 0, 0));
  Alarm.alarmRepeat(0, 0, 20, MorningOpen);
  Alarm.alarmRepeat(0, 1, 0, EveningClose );
  digitalWrite(ledPin, LOW);
}

void loop()
{
  // check if the pushbutton is pressed. If it is, the buttonState is LOW:
  if (digitalRead(upButtonPin) == LOW) {
    digitalClockDisplay();
    Serial.println("Opening door");
    startDoorMove(true);
    Alarm.delay(500); // debounce
    return;
  }
  
  if (digitalRead(downButtonPin) == LOW) {
    digitalClockDisplay();
    Serial.println("Closing door");
    startDoorMove(false);
    Alarm.delay(500); // debounce
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
  Alarm.delay(1);
}

