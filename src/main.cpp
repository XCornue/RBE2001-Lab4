#include <Arduino.h>
#include <Romi32U4.h>
#include "BlueMotor.h"
#include <servo32u4.h>
#include "claw.h"

BlueMotor motor;
Romi32U4ButtonA buttonA;
Romi32U4ButtonB buttonB;
Servo32U4Pin5 jawServo;
long timeToPrint = 0;
long now = 0;
long newPosition = 0;
long oldPosition = 0;
long sampleTime = 100;
int speedInRPM = 0;
int CPR = 270;
int motorEffort = 400;

/*enum SERVOSTATE
{
  CONTINUOUS,
  NOT_CONTINUOUS
};*/

void setup()
{
  Serial.begin(9600);
  motor.setup();
  motor.reset();
  jawServo.setMinMaxMicroseconds(0,20000);
  delay(3000);
  Serial.print("Effort");
  Serial.print("   ");
  Serial.println("Count");
  delay(3000);
}

void loop(){ //When button A is pressed, open. When button B is pressed, close. If stuck, open.
  if(buttonA.isPressed()){
    open(jawServo);
  }

  if(buttonB.isPressed()){
    close(jawServo);
  }
  
  Serial.println(analogRead(A0));
}
    

  // void loop(){
  //   //Continuous Servo Code
  //   if(buttonA.isPressed()){
  //     openContinuous(jawServo);
  //     delay(1000);
  //   }
  
  //   if(buttonB.isPressed()){
  //     closeContinuous(jawServo);
  //     delay(1000);
  //   }
  //   jawServo.writeMicroseconds(0);
  // }

  
  /*void loop(){
    jawServo.writeMicroseconds(servoOpen);
    delay(2000);

    linPositionVoltADC = analogRead(linPositionVoltADC);
    Serial.print("Intitial linPosition Volt ADC is ");
    Serial.print(linPositionVoltADC);

    if(buttonB.isPressed()){ //opens and closes when button is pressed, always opens first
      change();

      if(mode == 1){
        open();
      }

      if(mode == 0){
        close();
      }
    }
  }*/

// void loop() //use this code to test the dead band once the motor is in the gripper
// {
//   motor.deadBandTestPos();
//   //motor.deadBandTestNeg();
//   delay(4000);
//   motor.setEffort(0);
//   exit;
// }


// void loop() //this is what we ran when we had the move to function work perfect
// {
//   motor.moveTo(-540); 

//   motor.setEffort(0);
  
// }

// void loop() //this one to show the quadrature stuff working
// {
//     Serial.println(motor.getCount());
// }