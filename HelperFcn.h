#ifndef HelperFcn_H
#define HelperFcn_H

#include <LiquidCrystal.h>

// convert rpm to steps per second
int rpm2speed(float rpm, int stepPerRev, float gear);

// check if speed possible
int checkSpeed(int speed);

// convert steps per second to rpm
float speed2rpm(float stepPerSec, int stepPerRev, float gear);

// check for potentiometer update
bool detectChange(int currentSpeed, int previousSpeed, int treashold);

// print to LCD display
void printState(LiquidCrystal lcd, int desired, int actual, int stepsPerRev, int stat, float gear);


#endif
