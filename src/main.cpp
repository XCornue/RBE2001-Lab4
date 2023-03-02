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

//ultrasonic sensor variables
int position1 = 10;
int position2 = 15;
int position4 = 10;
int position7 = 15;

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
  WAITING,
  PATHA,
  PATHB
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
COURSE courseState = LINE_FOLLOWING;

void ISRsa(){
  if(robotState == PATHA){ //if using the non-continuous servo
    jawServo.writeMicroseconds(analogRead(A0)); //stop in place
  }
  if(robotState == PATHB){ //if using the continuous servo
    jawServo.writeMicroseconds(0); //stop in place
  }
  motorLeft.setMotorEffort(0); //stop moving
  motorRight.setMotorEffort(0);
  while(1==1){ //stay here until
    if(decoder.getKeyCode(remote0)){ //remotely brought out of the while loop
      break;
    }
  }
  
}

void setup()
{
  Serial.begin(9600);
  attachInterrupt(decoder.getKeyCode(remoteStopMode), ISRsa, RISING);
  motor.setup();
  motor.reset();
  jawServo.setMinMaxMicroseconds(0,20000);
  decoder.init();
  motorLeft.init();
  motorRight.init();
  delay(3000);
  Serial.println("ON");
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
    case WAITING: //waits to start until button press
    {
      turncount = 0;  //resets variables
      prevLocationWanted = 0;
      locationWanted = 2;
      openPosition = true;
      replacementComplete = false;
      while (decoder.getKeyCode() == remote1) { //if 1 is pressed
        open(jawServo); //open jaw and go on path A
        robotState = PATHA;
      }
      while (decoder.getKeyCode() == remote2){ //if 2 is pressed
        openContinuous(jawServo); //open jaw and go on path B
        robotState = PATHB;
      }
      break;
      }

    case PATHA: //path A (right side, non-continuous)
    {
      switch (courseState){
        case SERVO: //Handles the opening and closing of the jaw, know which to do based on a bool
          if(openPosition == true){
            close(jawServo);
            openPosition = false;
          }
          else if(openPosition == false){
            open(jawServo);
            openPosition = true;
          }
          if(prevLocationWanted == 4){ //if we go to servo when wanting to be in position 4, gp tp pausing
            courseState = PAUSING;
            break;
          }
          locationWanted = 2; //location wanted changes to 2 (line up position)
          courseState = BLUE_MOTOR;
        break;

        case BLUE_MOTOR: //controls the blue motor, based on where we are going and previous location at
          if(locationWanted == 2 && prevLocationWanted != 1){ //if going to location 2 and was at location 4
            motor.moveTo(clearance); //raise motor to clearance
            prevLocationWanted = locationWanted; //update locations
            locationWanted = 1;
            courseState = LINE_FOLLOWING;
          }

          else if(locationWanted == 1){ //if going to location 1
            prevLocationWanted = locationWanted; //update locations
            locationWanted = 2;
            motor.moveTo(twentyFive);  //lower to twentyFive angle
            courseState = SERVO;
          }
          
          else if(locationWanted == 4){ //if going to location 4
            prevLocationWanted = locationWanted; //update locations
            locationWanted = 2;
            motor.moveTo(flat); //lower to flat angle
            courseState = SERVO;
          }

          else{  //if at position 2 and was at location 1
            motor.moveTo(clearance); //raise motor to clearance
            prevLocationWanted = locationWanted; //update locations
            locationWanted = 4;
            courseState = LINE_FOLLOWING;
          }
        break;

        case LINE_FOLLOWING: //follows lines and stops when at location or at intersection
          followLine(motorLeft, motorRight); //follows the line using p control
          if (intersectionDetected(25)) { //when an intersection is detected, romi stops, waits 300ms, then switches to turning
            motorRight.setMotorEffort(0);//the count of the intersections determines which way the romi is to turn, determined in turn()
            motorLeft.setMotorEffort(0);
            timeUpdateCheck(300);
            courseState = TURNING;
            break;
          }

          if (ultrasonic.getDistance() == position1 && locationWanted == 1 && replacementComplete == false){
            //stops when going to location 1 for replacement
            courseState = BLUE_MOTOR;
            break;
          }

          if (ultrasonic.getDistance() == position2 && locationWanted == 2 && replacementComplete == false){
            //stops when going to location 2 for replacement
            courseState = BLUE_MOTOR;
            break;
          }

          if (ultrasonic.getDistance() == position4 && locationWanted == 4 && replacementComplete == false){
            //stops when going to location 4 for replacement
            courseState = BLUE_MOTOR;
            break;
          }

          if (ultrasonic.getDistance() == position4 && locationWanted == 4 && replacementComplete == true){
            //stops when going to location 4 for after replacement is complete
            courseState = TURNING;
            break;
          }

          if (ultrasonic.getDistance() == position7 && locationWanted == 7){
            //stops when going to location 7 to be swapped out
            robotState = WAITING;
            break;
          }

        break;

        case TURNING: //turns based on turncount and turnDirections
          turn(turncount, motorLeft, motorRight, turnDirectionsOne); //turns
          turncount++;
          if(turncount == 5){ //if turned 5 times, replacement is completed
            replacementComplete = true;
          }
          if(turncount == 7){ //after turning 7 times, go to the other side
            locationWanted = 7;
          }
        break;

        case PAUSING: //waiting for input
          if(decoder.getKeyCode() == remotePlayPause){ //when button is pressed, close servo and begin moving
            close(jawServo);
            openPosition = false;           
            locationWanted = 2;
            courseState = BLUE_MOTOR;
          }
        break;
        
        case MOVE_TO_LINE: //move until a line is found
          moveUntilLine(motorLeft, motorRight);
          courseState = TURNING;
        break;
      }
      break;
    }

    case PATHB: //very similar to PATHA, just using continuous servo and inverted turns
    {
      switch (courseState){
        case SERVO:
          if(openPosition == true){
            closeContinuous(jawServo);
            openPosition = false;
          }
          else if(openPosition == false){
            openContinuous(jawServo);
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

          if (ultrasonic.getDistance() == position1 && locationWanted == 1 && replacementComplete == false){
            courseState = BLUE_MOTOR;
            break;
          }

          if (ultrasonic.getDistance() == position2 && locationWanted == 2 && replacementComplete == false){
           courseState = BLUE_MOTOR;
            break;
          }

          if (ultrasonic.getDistance() == position4 && locationWanted == 4 && replacementComplete == false){
            courseState = BLUE_MOTOR;
            break;
          }

        if (ultrasonic.getDistance() == position4 && locationWanted == 4 && replacementComplete == true){
          courseState = TURNING;
          break;
        }

          if (ultrasonic.getDistance() == position7 && locationWanted == 7){
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
            locationWanted = 7;
          }
        break;

        case PAUSING:
          if(decoder.getKeyCode() == remotePlayPause){
            closeContinuous(jawServo);
            openPosition = false;            
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
  }
}