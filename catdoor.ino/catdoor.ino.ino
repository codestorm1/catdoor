/*
Adafruit Arduino - Lesson 16. Stepper
*/

#include <Stepper.h>

int in1Pin = 12;
int in2Pin = 11;
int in3Pin = 10;
int in4Pin = 9;

Stepper motor(512, in1Pin, in3Pin, in2Pin, in4Pin);  
int previous = 0;

void setup()
{
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);

  // this line is for Leonardo's, it delays the serial interface
  // until the terminal window is opened
  while (!Serial);
  
  Serial.begin(9600);
  motor.setSpeed(20);
}

void loop()
{
  if (Serial.available())
  {
    int steps = Serial.parseInt();
    Serial.println(steps);
    motor.step(steps);
    Serial.println("wait");
    delay(100);
  }
}

