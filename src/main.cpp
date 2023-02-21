#include <Arduino.h>
#include <Romi32U4.h>
#include "BlueMotor.h"
#include <servo32u4.h>


BlueMotor motor;
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

//Variables FIX VALUES!!!!
int servoClosed = 1000; //Length of Pulse in Microseconds for closing
int servoOpen = 1000; //Length of Pulse in Microseconds for opening
int servoStop = 1000; //Length of Pulse in Microseconds for stopping
int prevPositionVoltADC = 1000; //Whatever the ADC's previous value was
int linPositionVoltADC = 1000; //Whatever the ADC value is currently
int servoClosedVoltADC = 1000; //Whatever the ADC value is for when it is closed
int servoOpenVoltADC = 1000; //Whatever the ADC value is for when it is open
int printDelay = 500; //Delay of code
int mode = 2; //Mode 0 is closing, Mode 1 is opening, Mode 2 is starting

void setup()
{
  Serial.begin(9600);
  motor.setup();
  motor.reset();
  jawServo.attach();
  delay(3000);
  Serial.print("Effort");
  Serial.print("   ");
  Serial.println("Count");
  delay(3000);
}

void close(){ //closes the servo
  jawServo.writeMicroseconds(servoClosed); //starts closing
  while((servoClosedVoltADC - 5) <= linPositionVoltADC){ //when it is more open then goal
    if(prevPositionVoltADC == linPositionVoltADC){ //checks that we didn't get stuck
      open();
    }
    prevPositionVoltADC = linPositionVoltADC; //set for the check
    linPositionVoltADC = analogRead(linPositionVoltADC); //set for the check
  }
  jawServo.writeMicroseconds(servoStop); //stop when in position
}

void open(){ //opens the servo
  jawServo.writeMicroseconds(servoOpen); //starts opening
  while(linPositionVoltADC <= (servoOpenVoltADC + 5)){ //when it is more closed than goal
    linPositionVoltADC = analogRead(linPositionVoltADC); //sets for the check
  }
  jawServo.writeMicroseconds(servoStop); //stop when in position
}

void change(){ //just used to change between opening and closing
  if(mode == 1){
    mode = 0;
  }
  else{
    mode = 1;
  }
}

void loop(){
  jawServo.writeMicroseconds(servoStop); //don't know why it doesnt like writeMicroseconds, may have to ask a TA
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
}

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