
/*
 * Catdoor Controller 
 * 
 * from: Adafruit Arduino - Lesson 16. Stepper
*/

#include <Stepper.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

int in1Pin = 13;
int in2Pin = 12;
int in3Pin = 11;
int in4Pin = 10;
int ledPin = 13;

Stepper motor(512, in1Pin, in3Pin, in2Pin, in4Pin);  
int previous = 0;

bool doorIsOpen = true;

const int upButtonPin = 3; // button to open door, disables timer mode
const int timerButtonPin = 4; // button to turn on timer mode
const int downButtonPin = 5; // button to close door, also disables timer mode

int upButtonState = 0;
int downButtonState = 0;
int timerButtonState = 0;
bool timerMode = true;

int stepsPerRevolution = 2048;
int stepsPerPress = stepsPerRevolution * 6.25;

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

void moveDoor(bool open) {
  digitalWrite(ledPin, HIGH);
  int steps = open ? stepsPerPress : stepsPerPress * -1;
  motor.step(steps);
}

void openDoor() {
  if (!doorIsOpen) {
    Serial.println("Opening door");
    moveDoor(true);
    doorIsOpen = true;
    return;
  }
  Serial.println("Door is already open.");
}

void closeDoor() {
  if (doorIsOpen) {
    Serial.println("Closing door");
    moveDoor(false);
    doorIsOpen = false;
    return;
  }
  Serial.println("Door is already closed.");
}

void MorningOpen() {
  Serial.println("Open sez me");
  if (timerMode) {
    Serial.println("Time to open door");
    openDoor();
  }
}

void EveningClose() {
  Serial.println("Close sez me");
  if (timerMode) {
    Serial.println("Time to close door");
    closeDoor();
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
  
  Serial.begin(9600);
  Serial.println("starting up");
  motor.setSpeed(60);

  setTime(0,13,0,1,3,170); // set time to Saturday 8:29:00am Jan 1 2011
  digitalClockDisplay();
  
  Alarm.alarmRepeat(7, 0, 0, MorningOpen);  // 7am every day
  Alarm.alarmRepeat(20, 0, 0, EveningClose );  // 8pm every night
//  Alarm.alarmRepeat(0, 0, 20, MorningOpen);  // 7am every day
//  Alarm.alarmRepeat(0, 1, 0, EveningClose );  // 8pm every night
}

void loop()
{
  digitalWrite(ledPin, LOW);
  if (Serial.available()) // for debugging only
  {
    int steps = Serial.parseInt();
    Serial.println(steps);
    motor.step(steps);
    Alarm.delay(1000); // debounce
    return;
  }
  
  // check if the pushbutton is pressed. If it is, the buttonState is LOW:
  upButtonState = digitalRead(upButtonPin);
  if (upButtonState == LOW) {
    digitalClockDisplay();
    openDoor();
    timerMode = false;
    Alarm.delay(500); // debounce
    return;
  }
  
  downButtonState = digitalRead(downButtonPin);
  if (downButtonState == LOW) {
    digitalClockDisplay();
    closeDoor();
    timerMode = false;
    Alarm.delay(500); // debounce
    return;
  }

  timerButtonState = digitalRead(timerButtonPin);
  if (timerButtonState == LOW) {
    // switch to timer mode
    digitalClockDisplay();
    Serial.println("Timer mode enabled");
    timerMode = true;
    Alarm.delay(500); // debounce
    return;
  }
  Alarm.delay(10); // debounce
}

