#include <Arduino.h>
#include <Romi32U4.h>
#include "Romi32U4Motors.h"


// VARIABLES

const int lineRight = 36;
const int lineLeft = 39;

// STATS

const float track = 5.78;
const float diameter = 2.75;

const float defaultSpeed = 300;   //was 360
const float slowSpeed = 120;

// FUNCTIONS----------------------------------------------------------------------------


// TIMER---------------------------------------------------------------------------------

bool timeUpdateCheck(int samplePeriod) {    //mock delay() function this returns bool and allows me to change sample period for each function its called in
  int beginTime = millis();                 //after the elapsed time is equal to the sample period, used often
  unsigned long currentTime = millis();
  while (samplePeriod >= currentTime - beginTime) {
    currentTime = millis();     //updates so that current time will get bigger every time its checked in the while loop
  }
  return true; //it only returns true after time is elapsed, can not return false
}//Matt

// LINE FOLLOWING GROUP---------------------------------------------------------------------


float piControl(int target, int actual, float Kp, float Ki) {     //proportional and intergral control to assist line following
  static float errorSum = 0;                                      //we did not end up using integral control, but if we included
  static float lastTime = 0;                                      //it here and did not need it then we could easily set ki to 0
  static float effort = 0;
  float samplePeriod = 50;
  static float maxSum = 2000;
  unsigned long now = millis();
  if (now - lastTime >= samplePeriod) {
    float error = target - actual;   //error
    errorSum += error ;                //Intergral Term
    if (errorSum > maxSum) errorSum = maxSum;
    if (errorSum < -maxSum) errorSum = -maxSum;


    effort = Kp * error + Ki * errorSum;    //Calculation for Pi effort
    lastTime = now;
    return effort;
  }
  return effort;
}//Matt

void followLine(LeftMotor motorLeft, RightMotor motorRight) {
  static float Kp = .07; // even this low there is occasional oscillation
  static float Ki = 0;
  static int targetADC = 0; // total reflection

  int left = analogRead(lineLeft);
  int right = analogRead(lineRight);

  float motorEffort = piControl(targetADC, (left - right), Kp, Ki); // - means turn left, + means turn right

  if (motorEffort < 0) { // turn left
    motorLeft.setMotorEffort(defaultSpeed + motorEffort); // left forward and right backwards simultaneously for better error correction
    motorRight.setMotorEffort(defaultSpeed - motorEffort);
  } else if (motorEffort > 0) { //turn right
    motorLeft.setMotorEffort(defaultSpeed + motorEffort);
    motorRight.setMotorEffort(defaultSpeed - motorEffort);
  } else { // go straight
    motorLeft.setMotorEffort(defaultSpeed);
    motorRight.setMotorEffort(defaultSpeed);
  }
}//Martin

//TURN HELPERS----------------------------------------------------------------------------


void lineCorrect(LeftMotor motorLeft, RightMotor motorRight) {                                          //currently not called, experiment to correct overshoot of k turns
  static float Kp = .07;                                      //uses p control, only would work if line sensor elements are very close to to the width of the tape
  static float Ki = 0;                                        //tried to avoid dead reckoning as much as possible
  static int targetADC = 0; // total reflection

  int left = analogRead(lineLeft);
  int right = analogRead(lineRight);

  float motorEffort = piControl(targetADC, (left - right), Kp, Ki);

  if (motorEffort < 0) { // turn left
    motorLeft.setMotorEffort(0);
    motorRight.setMotorEffort(-motorEffort);
  } else if (motorEffort > 0) { //turn right
    motorLeft.setMotorEffort(motorEffort);
    motorRight.setMotorEffort(0);
  } else if (0 == motorEffort){
    return;
  }
}//Matt


bool turnLeftHelper() {                                       //checks if right element detects line, compares to prev value, name convention since this is used in turnLeft()
  bool returnValue = false;
  static bool previousRight = false;
  bool currentRight = false;
  int right = analogRead(lineRight);
  if (right > 2000) {
    currentRight = true;
  }
  if (currentRight == true && previousRight == false) {
    returnValue = true;
  }
  previousRight = currentRight;
  return returnValue;
}//Matt


bool turnRightHelper(int samplePeriod) {                     //checks if left element detects line, compares to prev value, name convention since this is used in turnRight()
  bool returnValue = false;
  static bool previousLeft = false;
  bool currentLeft = false;
  int left = analogRead(lineLeft);
  if (left > 2000) {
    currentLeft = true;
  }
  if (currentLeft == true && previousLeft == false) {
    returnValue = true;
  }
  previousLeft = currentLeft;
  return returnValue;
}//Matt


bool reverseDirectionHelper(int samplePeriod) {              //exact same as turnLeftHelper, different name to help comprehension between team members
  bool returnValue = false;
  static bool previousRight = false;
  bool currentRight = false;
  int right = analogRead(lineRight);
  if (right > 2000) {
    currentRight = true;
  }
  if (currentRight == true && previousRight == false) {
    returnValue = true;
  }
  previousRight = currentRight;
  return returnValue;
}//Matt

// LINE DETECTION GROUP-----------------------------------------------------------------

bool lineDetectedRight() { // detects line on right sensor
  bool returnValue = false;
  static bool previousRight = false;
  bool currentRight = false;
  int right = analogRead(lineRight);
  if (right > 1000) {
    currentRight = true;
  }
  if (currentRight == true && previousRight == false) {
    returnValue = true;
  }
  previousRight = currentRight;
  return returnValue;
}//Martin

bool lineDetectedLeft() { // detects line on left sensor
  bool returnValue = false;
  static bool previousLeft = false;
  bool currentLeft = false;
  int left = analogRead(lineLeft);
  if (left > 1000){
    currentLeft = true;
  }
  if (!previousLeft && currentLeft) { //line detected
    returnValue = true;
  }
  previousLeft = currentLeft;
  return returnValue;
}//Martin

void moveUntilLine(LeftMotor motorLeft, RightMotor motorRight) {    //romi runs until either the left or right sensor reads a line
  timeUpdateCheck(600);   //Used for when need to find line after third bag, but perpendicular is not guarenteed, detectIntersection would not work
  motorRight.setMotorEffort(defaultSpeed);
  motorLeft.setMotorEffort(defaultSpeed);
  while (!lineDetectedRight() || !lineDetectedLeft()) {}
  motorRight.setMotorEffort(0);
  motorLeft.setMotorEffort(0);
}//Matt

// TURNING GROUP ------------------------------------------------------------------------------------
//basic premise of turning functions is that they will run motors at some speed (depending how it wants to turn)
//and when it runs, it checks to see if a sensor is on a line. For example, to turn left, the right sensor element "catches" the line, 
//allowing the romi to be on the line to continue line following


void turnLeft(int samplePeriod, LeftMotor motorLeft, RightMotor motorRight) {         //turns left by setting right motor to default speed and left motor to zero
  motorRight.setMotorEffort(defaultSpeed);      //this alligns line sensor with line
  motorLeft.setMotorEffort(0);               
  timeUpdateCheck(samplePeriod);
  while (!turnLeftHelper()) {}
  motorRight.setMotorEffort(0);
  motorLeft.setMotorEffort(0);
}//Matt

void turnRight(int samplePeriod, LeftMotor motorLeft, RightMotor motorRight) {        //turns right by setting left motor to default speed and right motor to zero
  motorRight.setMotorEffort(0);                 //this alligns line sensor with line
  motorLeft.setMotorEffort(defaultSpeed);
  timeUpdateCheck(samplePeriod);
  while (!turnLeftHelper()) {}
  motorRight.setMotorEffort(0);
  motorLeft.setMotorEffort(0);
}//Matt

void reverseDirection(int samplePeriod, LeftMotor motorLeft, RightMotor motorRight) { //rotates around center axis CCW
  motorLeft.setMotorEffort(-defaultSpeed / 2);
  motorRight.setMotorEffort(defaultSpeed / 2);
  timeUpdateCheck(samplePeriod);
  while (!turnLeftHelper()) {}
  motorRight.setMotorEffort(0);
  motorLeft.setMotorEffort(0);
}//Matt

void turnRightReverse(int samplePeriod, LeftMotor motorLeft, RightMotor motorRight) {   //tunrns CW 90 degrees by setting right motor to negative default speed and left motor to zero
  motorRight.setMotorEffort(-defaultSpeed);       //this alligns line sensor perpendicular to line
  motorLeft.setMotorEffort(0);
  timeUpdateCheck(samplePeriod);
  while (!turnLeftHelper()) {}
  motorRight.setMotorEffort(0);
  motorLeft.setMotorEffort(0);
}//Matt

void turnLeftReverse(int samplePeriod, LeftMotor motorLeft, RightMotor motorRight) {   //tunrns CCW 90 degrees by setting right motor to zero and left motor to negative default speed 
  motorRight.setMotorEffort(0);                  //this alligns line sensor perpendicular to line
  motorLeft.setMotorEffort(-defaultSpeed);
  timeUpdateCheck(samplePeriod);
  while (!turnLeftHelper()) {}
  motorRight.setMotorEffort(0);
  motorLeft.setMotorEffort(0);
}//Matt

void turn(int turnCount, LeftMotor motorLeft, RightMotor motorRight, int instructions[9]) {
  if (turnDirection[turnCount] == 0) {        //left, only power to right motor
    turnLeft(400, motorLeft, motorRight);
    return;
  } else if (turnDirection[turnCount] == 2) {  //right, only power to left motor
    turnRight(400, motorLeft, motorRight);
    return;
  } else if (turnDirection[turnCount] == 3) {  //around CCW, half negative power to left motor, half positive power to right motor
    reverseDirection(600, motorLeft, motorRight);
    timeUpdateCheck(100);
    return;
  } else if (turnDirection[turnCount] == 4) {  //K turn, CCW
    turnRightReverse(400, motorLeft, motorRight);                             //negative speed to right motor until right reads line, then turn right till line
    turnRight(400, motorLeft, motorRight);                                    //this allows us to separate a consistent distance away from bags and delivery zones
    motorRight.setMotorEffort(0);                            
    motorLeft.setMotorEffort(defaultSpeed);
    timeUpdateCheck(120);               //slight dead reckoning, would work perfect if line had no thickness, but since it does, the robot will not make exact 180 turn, more like 175
    motorRight.setMotorEffort(0);
    motorLeft.setMotorEffort(0);
    timeUpdateCheck(100);
    return;
  }
  else if (turnDirection[turnCount] == 5) {  //K turn, CW, not used, written in case testing might require opposite k turn
    turnLeftReverse(400, motorLeft, motorRight);                            //negative speed to left until left reads line, then turn left till line
    turnLeft(400, motorLeft, motorRight);
    return;
  } else if (turnDirection[turnCount] == 6) {  //turn right 90, negative speed to right motor until left sensor detects line
    turnRightReverse(400, motorLeft,  motorRight);
    return;
  } else if (turnDirection[turnCount] == 1) {  //straight
    motorLeft.setMotorEffort(defaultSpeed);
    motorRight.setMotorEffort(defaultSpeed);
    timeUpdateCheck(120);               //skip over line, since only called at 4 point intersection it had to drive over the intersection without detecting it again
    motorLeft.setMotorEffort(0);              //dead reckoning necessary since, depending on surface, speed, and presence of bag, momentum could carry romi over intersection, meaning a detect intersection gone function wouldnt always work
    motorRight.setMotorEffort(0);
    return;
  } else if (turnDirection[turnCount] == 6) { //since this will never run without an intersection detected, this isn't needed but its there just in case
    return;
  }
}//Matt

// INTERSECTION GROUP--------------------------------------------------------------------------------------------------

// INTERSECTION GROUP

bool intersectionDetected(int samplePeriod) { // takes in a sample period
  bool returnValue = false;
  static bool previousInt = false;
  bool currentInt = false;
  int left = analogRead(lineLeft);
  int right = analogRead(lineRight);
  if (left > 1900 && right > 1900) { // if both are over tape
    currentInt = true;
  }
  if (currentInt && !previousInt) { // new intersection detected
    if (timeUpdateCheck(samplePeriod)) {
      left = analogRead(lineLeft);
      right = analogRead(lineRight);
      if (left > 1900 && right > 1900) {
        returnValue = true;
      }
    }
  }
  previousInt = currentInt;
  return returnValue;
}//Martin