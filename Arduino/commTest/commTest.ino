/*      Command byte:
	|  2 bits  |  2 bits  |  4 bits  |
	|   SIG    |  OPCODE  | ARGUMENT |

	SIG is the signature for the communication.
	OPCODE is a 2 bit unsigned int.
	ARGUMENT is a 4 bit unsigned int.

	OPCODES
	0 | Left Power Motor  | Arguments = 0-15 (7-8: STOP)|
	1 | Right Power Motor | Arguments = 0-15 (7-8: STOP)|
	2 | Rotational Motor  | Arguments = 0-14      	    |
	3 | Kicker            | Arguments = 1 to fire 	    |
*/

#include "SDPArduino.h"
#include <Wire.h>

byte msg;  // the command buffer
byte SIG = 0b11000000;

/* BIT MASKS FOR OPCODES */
byte KICKER_MASK = 0b00110000;
byte ROT_MASK = 0b00100000;
byte RIGHT_MOTOR_MASK = 0b00010000;
byte LEFT_MOTOR_MASK = 0b00000000;

/* Timed action */
unsigned long time = millis();

boolean kickerAction = false;
unsigned long kickerTime = millis();

void setup()
{
  SDPsetup(); 
  Serial.println("Robot started");
}

int getArg(byte msg)
{
  return (int)(msg & 0b00001111);
}

void kickerStop()
{
    kickerAction = false;
    motorStop(2);
}

void controlKicker(int value)
{
  if(value == 0)
  {
    /* Grab */
    kickerTime = millis() + 500;
    kickerAction = true;
    motorForward(2,100);
  }
  else if(value == 1)
  {
    /* Kick */
    kickerTime = millis() + 500;
    kickerAction = true;
    motorBackward(2,100);
  }
}

void controlMotor(int motor, byte msg)
{
  int motorGear = getArg(msg);
  
  if(motorGear == 7 || motorGear == 8)
  {
     motorStop(motor);
  }
  else
  {
    if(motorGear > 7)
    {
      motorGear = 15 - motorGear;
      int motorSpeed = 100 - ((motorGear * 100) / 7);
      motorForward(motor, motorSpeed);
    }
    else    
    {
      int motorSpeed = 100 - ((motorGear * 100) / 7);
      motorBackward(motor, motorSpeed);
    }
  }
}

void loop()
{
  /* Update time and check if action is needed */
  time = millis();
  
  /* If the kicker flag kickerAction is set,
  check if the time is reached. */
  if(kickerAction && (kickerTime < time))
  {
    kickerStop();
  }
}

void serialEvent() {
 if (Serial.available()>0) // character received
  {
    msg = Serial.read();
    
    //check if it's our message
    if((msg & SIG) == SIG)
    {
      Serial.write(msg);
      //Serial.println(KICKER_MASK, BIN);
      //Serial.println((msg & KICKER_MASK), BIN);
     
     if((msg & KICKER_MASK) == KICKER_MASK)
     {
      // Serial.println("KICKER");
      controlKicker(getArg(msg));
     }
     else if((msg & ROT_MASK) == ROT_MASK)
     {
       //Serial.println("ROTATION");
     }
     else if((msg & RIGHT_MOTOR_MASK) == RIGHT_MOTOR_MASK)
     {        
       //Serial.println("MOTOR");
       controlMotor(0, msg);
     }   
     else if((msg & LEFT_MOTOR_MASK) == LEFT_MOTOR_MASK)
     {
       //Serial.println("LED");
       controlMotor(1, msg);
     }
    }
  }
}


