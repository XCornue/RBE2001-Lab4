#include <Arduino.h>
#include <Romi32U4.h>
#include "BlueMotor.h"
#include <servo32u4.h>
#include "claw.h"
#include "IRdecoder.h"
#include "remoteconstants.h"
#include "Linefollowing.h"
#include "Romi32U4Motors.h"
#include "Rangefinder.h"

BlueMotor motor;
Romi32U4ButtonA buttonA;
Romi32U4ButtonB buttonB;
Servo32U4Pin5 jawServo;
IRDecoder decoder(14);
LeftMotor motorLeft;
RightMotor motorRight;
Rangefinder ultrasonic(17, 12);

//line following variables
int motorEffort = 400;

//where we want the blue motor
int clearance = 1000;
int twentyFive = 800;
int fourtyFive = 900;
int flat = 500;

// LOGIC VARIABLES
// 0 = left, 1 = straight, 2 = right, 3 = around CCW, 4 = K turn CCW, 5 = K turn CC, 6 = turn 90 right, 7 = blank
int turnDirectionsOne[9] = {3, 0, 3, 2, 3, 0, 0, 0, 0};  //array for demo, has correct values
int turnDirectionsTwo[9] = {3, 2, 3, 1, 3, 2, 2, 2, 2};

//Tells us the state of the machine
int turncount = 0;
int prevLocationWanted = 0;
int locationWanted = 2;
bool openPosition = true;
bool replacementComplete = false;
int step = 0;

enum ROBOT_STATE{
  START_1,
  START_2,
  RESUME,
  WAITING
};

enum COURSE{
  SERVO,
  BLUE_MOTOR,
  LINE_FOLLOWING,
  TURNING,
  PAUSING,
  MOVE_TO_LINE
};

ROBOT_STATE robotState = WAITING;
COURSE courseState = PAUSING;

void setup()
{
  Serial.begin(9600);
  motor.setup();
  motor.reset();
  jawServo.setMinMaxMicroseconds(0,20000);
  decoder.init();
  motorLeft.init();
  motorRight.init();
  delay(3000);
  Serial.print("Effort");
  Serial.print("   ");
  Serial.println("Count");
  delay(3000);
}

// void loop(){ //When button A is pressed, open. When button B is pressed, close. If stuck, open.
//   if(buttonA.isPressed()){
//     open(jawServo);
//   }

//   if(buttonB.isPressed()){
//     close(jawServo);
//   }
// }
    

  void loop(){
    int16_t code = decoder.getKeyCode();
    switch(code){
      case remote1: //Open non-continuous servo when button 1 is pressed
      open(jawServo);
      break;
      case remote2: //Close non-continuous servo when button 2 is pressed
      close(jawServo);
      break;
      case remote3: //Open continuous servo when button 3 is pressed
      openContinuous(jawServo);
      break;
      case remote4: //Close continuous servo when button 4 is pressed
      closeContinuous(jawServo);
      break;
      }
    }

  
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

void updateStateMachine() {
  switch (robotState) {
    case WAITING:                           //waits to start until button press
      int turncount = 0;
      int prevLocationWanted = 0;
      int locationWanted = 2;
      bool openPosition = true;
      bool replacementComplete = false;
      while (decoder.getKeyCode() == remote1) {
        open(jawServo);
        robotState = START_1;
      }
      while (decoder.getKeyCode() == remote2){
        openContinuous(jawServo);
        robotState = START_2;
      }
      while (decoder.getKeyCode() == remote3){
        robotState = RESUME;
      }
      break;

    case START_1:
      switch (courseState){
        case SERVO:
          if(openPosition == true){
            close(jawServo);
            openPosition = false;
          }
          else if(openPosition == false){
            open(jawServo);
            openPosition = true;
          }
          if(prevLocationWanted == 4){
            courseState = PAUSING;
            break;
          }
          locationWanted = 2;
          courseState = BLUE_MOTOR;
        break;

        case BLUE_MOTOR:
          if(locationWanted == 2 && prevLocationWanted != 1){
            motor.moveTo(clearance);
            prevLocationWanted = locationWanted;
            locationWanted = 1;
            courseState = LINE_FOLLOWING;
          }

          else if(locationWanted == 1){
            prevLocationWanted = locationWanted;
            locationWanted = 2;
            motor.moveTo(twentyFive);
            courseState = SERVO;
          }
          
          else if(locationWanted == 4){
            prevLocationWanted = locationWanted;
            locationWanted = 2;
            motor.moveTo(flat);
            courseState = SERVO;
          }

          else{
            motor.moveTo(clearance);
            prevLocationWanted = locationWanted;
            locationWanted = 4;
            courseState = LINE_FOLLOWING;
          }
        break;

        case LINE_FOLLOWING:
          followLine(motorLeft, motorRight);                         //follows the line using p control
          if (intersectionDetected(25)) {       //when an intersection is detected, romi stops, waits 300ms, then switches to turning
            motorRight.setMotorEffort(0);             //the count of the intersections determines which way the romi is to turn, determined in turn()
            motorLeft.setMotorEffort(0);
            timeUpdateCheck(300);
            courseState = TURNING;
            break;
          }

          if (ultrasonic.getDistance() == 10 && locationWanted == 1 && replacementComplete == false){
            courseState = BLUE_MOTOR;
            break;
          }

          if (ultrasonic.getDistance() == 10 && locationWanted == 2 && replacementComplete == false){
            courseState = BLUE_MOTOR;
            break;
          }

          if (ultrasonic.getDistance() == 10 && locationWanted == 4 && replacementComplete == false){
            courseState = BLUE_MOTOR;
            break;
          }

          if (ultrasonic.getDistance() == 10 && locationWanted == 4 && replacementComplete == true){
            courseState = TURNING;
            break;
          }

          if (ultrasonic.getDistance() == 10 && locationWanted == 7){
            robotState = WAITING;
            break;
          }

        break;

        case TURNING:
          turn(turncount, motorLeft, motorRight, turnDirectionsOne);
          turncount++;
          if(turncount == 5){
            replacementComplete = true;
          }
          if(turncount == 7){
            locationWanted == 7;
          }
        break;

        case PAUSING:
          if(decoder.getKeyCode() == remotePlayPause){
            close(jawServo);           
            locationWanted = 2;
            courseState = BLUE_MOTOR;
          }
        break;
        
        case MOVE_TO_LINE:
          moveUntilLine(motorLeft, motorRight);
          courseState = TURNING;
        break;
      }
      break;

    case START_2:
      switch (courseState){
        switch (courseState){
          case SERVO:
            if(openPosition == true){
              close(jawServo);
              openPosition = false;
            }
            else if(openPosition == false){
              open(jawServo);
              openPosition = true;
            }
            if(prevLocationWanted == 4){
              courseState = PAUSING;
              break;
            }
            locationWanted = 2;
            courseState = BLUE_MOTOR;
          break;

          case BLUE_MOTOR:
            if(locationWanted == 2 && prevLocationWanted != 1){
              motor.moveTo(clearance);
              prevLocationWanted = locationWanted;
              locationWanted = 1;
              courseState = LINE_FOLLOWING;
            }

            else if(locationWanted == 1){
              prevLocationWanted = locationWanted;
              locationWanted = 2;
              motor.moveTo(twentyFive);
              courseState = SERVO;
            }
          
            else if(locationWanted == 4){
              prevLocationWanted = locationWanted;
              locationWanted = 2;
              motor.moveTo(flat);
              courseState = SERVO;
            }

            else{
              motor.moveTo(clearance);
              prevLocationWanted = locationWanted;
              locationWanted = 4;
              courseState = LINE_FOLLOWING;
            }
          break;

          case LINE_FOLLOWING:
            followLine(motorLeft, motorRight);                         //follows the line using p control
            if (intersectionDetected(25)) {       //when an intersection is detected, romi stops, waits 300ms, then switches to turning
              motorRight.setMotorEffort(0);             //the count of the intersections determines which way the romi is to turn, determined in turn()
              motorLeft.setMotorEffort(0);
              timeUpdateCheck(300);
              courseState = TURNING;
              break;
            }

            if (ultrasonic.getDistance() == 10 && locationWanted == 1 && replacementComplete == false){
              courseState = BLUE_MOTOR;
              break;
            }

            if (ultrasonic.getDistance() == 10 && locationWanted == 2 && replacementComplete == false){
             courseState = BLUE_MOTOR;
              break;
            }

            if (ultrasonic.getDistance() == 10 && locationWanted == 4 && replacementComplete == false){
              courseState = BLUE_MOTOR;
              break;
            }

          if (ultrasonic.getDistance() == 10 && locationWanted == 4 && replacementComplete == true){
            courseState = TURNING;
            break;
          }

            if (ultrasonic.getDistance() == 10 && locationWanted == 7){
              robotState = WAITING;
              break;
            }

          break;

          case TURNING:
            turn(turncount, motorLeft, motorRight, turnDirectionsTwo);
            turncount++;
            if(turncount == 5){
              replacementComplete = true;
            }
            if(turncount == 7){
              locationWanted == 7;
            }
          break;

          case PAUSING:
            if(decoder.getKeyCode() == remotePlayPause){
              close(jawServo);           
              locationWanted = 2;
              courseState = BLUE_MOTOR;
            }
          break;
        
          case MOVE_TO_LINE:
            moveUntilLine(motorLeft, motorRight);
            courseState = TURNING;
          break;
        }
        break;
      }
      break;

    case RESUME:
      
      break;
  }
}