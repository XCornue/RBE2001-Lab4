#include <Arduino.h>
#include <BlueMotor.h>
#include <Romi32U4.h>
#include <math.h>
#include <servo32u4.h>

//Variables FIX VALUES!!!!
float servoClosed = 2250; //2500; //Length of Pulse in Microseconds for closing
float servoOpen = 1500; //750; //Length of Pulse in Microseconds for opening
int doubleprevPositionVoltADC = -2;
int prevPositionVoltADC = -1; //Whatever the ADC's previous value was
int linPositionVoltADC = 0; //Whatever the ADC value is currently
int servoClosedVoltADC = 422; //Whatever the ADC value is for when it is closed
int prevPositionContinuous = -2;
int linPositionContinuous = -1;

void open(Servo32U4Pin5 claw){ //opens the servo
  claw.writeMicroseconds(servoOpen); //starts opening
  delay(200); //waits so nothing can interrupt the servo
}

void close(Servo32U4Pin5 claw){ //closes the servo
  claw.writeMicroseconds(servoClosed); //starts closing
  while((servoClosedVoltADC - 5) >= linPositionVoltADC){ //when it is more open then goal
    doubleprevPositionVoltADC = prevPositionVoltADC; //the double previous is set to the value of the previous
    prevPositionVoltADC = linPositionVoltADC; //the previous is set to the value of the current
    delay(20); //wait for position to change
    linPositionVoltADC = analogRead(A0); //the current is set to the value of the A0 pin
    if((prevPositionVoltADC == linPositionVoltADC) && (doubleprevPositionVoltADC == linPositionVoltADC)){ //checks that we didn't get stuck
      open(claw); //opens claw
      break;
    }
  }
  linPositionVoltADC = 0; //resets the variable
  prevPositionVoltADC = -1; //resets the variable
  doubleprevPositionVoltADC = -2; //resets the variable
}

  //Continuous Servo Code

void openContinuous(Servo32U4Pin5 claw){
  claw.writeMicroseconds(500); //set the servo to opening
  while(analogRead(A0) > 10){ // wait until the linear potentiometer reads < 10
  }
  claw.writeMicroseconds(0); //set the servo to stopping
}

void closeContinuous(Servo32U4Pin5 claw){
  claw.writeMicroseconds(2000); //set the servo to closing
  while(analogRead(A0) <= 510){ // wait until the linear potentiometer reads >= 510
    Serial.println(analogRead(A0)); //for testing purposes
    linPositionContinuous = analogRead(A0); //set the current position to the linear potentiometer reading
    if(prevPositionContinuous == linPositionContinuous){ //if the current position equals the last position 
      openContinuous(claw); //open the claw
      break;
    }
    prevPositionContinuous = linPositionContinuous; //set previous position to current position
    delay(1000); //wait for a delay
  }
  claw.writeMicroseconds(0); //set the servo to stopping
}