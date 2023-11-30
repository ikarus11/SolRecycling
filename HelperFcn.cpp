#include "HelperFcn.h"
#include <Arduino.h>
#include <LiquidCrystal.h>

// convert rpm to steps per second
int rpm2speed(float rpm, int stepPerRev) {
    return rpm/60*stepPerRev;
}

// check speed
int checkSpeed(int speed) {
    int _max_speed = 900; // 10% reserve (1000 max for arduino)
    if (speed > _max_speed) {
        return _max_speed;
    } else {
        return speed;
    }
}

// convert steps per second to rpm
float speed2rpm(float stepPerSec, int stepPerRev) {
    return stepPerSec/stepPerRev*60.0;
}

// check for potentiometer update
bool detectChange(int currentSpeed, int previousSpeed, int treashold) {
    if (abs(previousSpeed - currentSpeed) > treashold) {
        return true;
    } else {
        return false;
    }
}

// print to LCD display
void printState(LiquidCrystal lcd, int desired, int actual, int stepsPerRev, int stat){
    // Calculate desired and actual speeds
    float desiredSpeed  = speed2rpm(desired, stepsPerRev);
    float actualSpeed   = speed2rpm(actual, stepsPerRev);

    // Move the cursor to the beginning of the line and overwrite the content
    switch (stat) {
        case 0:
            lcd.setCursor(0, 0);
            lcd.print("SLEEP MODE   ");
            lcd.setCursor(0, 1);
            lcd.print(actualSpeed);
            lcd.setCursor(7,1);
            lcd.print("RPM");
            break;
        case 10:
            lcd.setCursor(0, 0);
            lcd.print("RUNNING     ");
            lcd.setCursor(0, 1);
            lcd.print(actualSpeed);
            lcd.setCursor(7,1);
            lcd.print("RPM");
            break;
        case 20:
            lcd.setCursor(0, 0);
            lcd.print("STOPPING    ");
            lcd.setCursor(0, 1);
            lcd.print(actualSpeed);
            lcd.setCursor(7,1);
            lcd.print("RPM");
            break;
        default:
            break;
    }
}
