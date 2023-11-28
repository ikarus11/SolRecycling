#ifndef HelperFcn_H
#define HelperFcn_H

#include <LiquidCrystal.h>

// convert rpm to steps per second
int rpm2speed(float rpm, int stepPerRev);

// convert steps per second to rpm
float speed2rpm(int stepPerSec, int stepPerRev);

// check for potentiometer update
bool detectChange(int currentSpeed, int previousSpeed, int treashold);

// print to LCD display
void printState(LiquidCrystal lcd, int desired, int actual, int stepsPerRev, int stat);


#endif
