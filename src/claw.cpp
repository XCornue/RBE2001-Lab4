#include <Arduino.h>
#include <BlueMotor.h>
#include <Romi32U4.h>
#include <math.h>
#include <servo32u4.h>

//Variables FIX VALUES!!!!
float servoClosed = 2500; //Length of Pulse in Microseconds for closing
float servoOpen = 750; //Length of Pulse in Microseconds for opening
int doubleprevPositionVoltADC = -2;
int prevPositionVoltADC = -1; //Whatever the ADC's previous value was
int linPositionVoltADC = 0; //Whatever the ADC value is currently
int servoClosedVoltADC = 437; //Whatever the ADC value is for when it is closed

void open(Servo32U4Pin5 claw){ //opens the servo
  claw.writeMicroseconds(servoOpen); //starts opening
  delay(200);
}

void close(Servo32U4Pin5 claw){ //closes the servo
  claw.writeMicroseconds(servoClosed); //starts closing
  while((servoClosedVoltADC - 5) >= linPositionVoltADC){ //when it is more open then goal
    doubleprevPositionVoltADC = prevPositionVoltADC;
    prevPositionVoltADC = linPositionVoltADC; //set for the check
    delay(20);
    linPositionVoltADC = analogRead(A0); //set for the check
    if((prevPositionVoltADC == linPositionVoltADC) && (doubleprevPositionVoltADC == linPositionVoltADC)){ //checks that we didn't get stuck
      open(claw);
      break;
    }
  }
  linPositionVoltADC = 0;
  prevPositionVoltADC = -1;
  doubleprevPositionVoltADC = -2;
}

  //Continuous Servo Code
void closeContinuous(Servo32U4Pin5 claw){
  claw.writeMicroseconds(2000);
  delay(175);
  claw.writeMicroseconds(0);
}

void openContinuous(Servo32U4Pin5 claw){
  claw.writeMicroseconds(1000);
  delay(175);
  claw.writeMicroseconds(0);
}