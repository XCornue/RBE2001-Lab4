#include <Arduino.h>
#include <Romi32U4.h>
#include "BlueMotor.h"


BlueMotor motor;
Romi32U4ButtonB buttonB;
long timeToPrint = 0;
long now = 0;
long newPosition = 0;
long oldPosition = 0;
long sampleTime = 100;
int speedInRPM = 0;
int CPR = 270;
int motorEffort = 400;

void setup()
{
  Serial.begin(9600);
  motor.setup();
  motor.reset();
  delay(3000);
  Serial.print("Effort");
  Serial.print("   ");
  Serial.println("Count");
  delay(3000);
}


// void loop() //use this code to test the dead band once the motor is in the gripper
// {
//   motor.deadBandTestPos();
//   //motor.deadBandTestNeg();
//   delay(4000);
//   motor.setEffort(0);
//   exit;
// }


void loop() //this is what we ran when we had the move to function work perfect
{
  motor.moveTo(-540); 

  motor.setEffort(0);
  
}

// void loop() //this one to show the quadrature stuff working
// {
//     Serial.println(motor.getCount());
// }