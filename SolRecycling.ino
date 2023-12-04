#include <AccelStepper.h>
#include <MultiStepper.h>
#include <LiquidCrystal.h>

/* --- CLASSES --- */
#include "Timer.h"

/* --- FUNCTIONS ---*/
#include "HelperFcn.h"

/* --- STEPPER --- */
// Define the stepper motor connections
#define EN_PIN      4
#define STEP_PIN    5
#define DIR_PIN     6

/* INPUTS */
#define ANALOG_IN A7 // A0 for UNO A7 for NANO

// Create an AccelStepper object
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
// stepper variables
const int steps_p_rev   = 400;  // steps per revolution (max 150 rpm)
const float gear_ratio  = 1;    // driven / driving

/* --- LCD DISPLAY --- */
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
//const int rs = 11, en = 12, d4 = 5, d5 = 6, d6 = 7, d7 = 8;
//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/* --- TIMERS --- */
Timer timer_update_speed(100);    // read potetiometer every 100 ms
Timer timer_update_lcd(1000);     // lcd display update every 300 ms

/* VARIABLES */
const int speed_max     = checkSpeed(rpm2speed(125, steps_p_rev, gear_ratio));  // Vorgabe 120 rpm
const int speed_min     = rpm2speed(20, steps_p_rev, gear_ratio);               // Vorgabe 30 rpm

// TODO: adjust values
const float acceleration_SI = 5;               // 1/s^2 
const int acceleration      = acceleration_SI;  // steps/s^2

/* STATE MACHINE */
int state           = 0;
int speed           = 0;
int desired_speed   = 0;

/* SIGNALS HANDLING */
const int hysterese = 5;    // steps per second

/* MONITORING */
float speed_rpm = speed2rpm(speed, steps_p_rev, gear_ratio);

/* SETUP */
void setup() {
    // Stepper configuration
    stepper.setMaxSpeed(speed_max);             // Set the maximum speed in steps per second
    stepper.setAcceleration(acceleration);      // Set the acceleration in steps per second per second    
    stepper.setCurrentPosition(0);              // Set the initial position
    stepper.setEnablePin(EN_PIN);               // Set en/disable pin of driver
    stepper.setPinsInverted(false,false,false); // DIRECTION/STEP/ENABLE
    stepper.disableOutputs();                   // diable current to stepper
    
    // Set the initial states
    state           = 0;
    
    // enable serial communication
    Serial.begin(115200);
    
    // Timers
    timer_update_speed.start();
    timer_update_lcd.start();

    // LED configuration
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // set up the LCD's number of columns and rows:
    // lcd.begin(16, 2);
    
    // printState(lcd, desired_speed, speed, steps_p_rev, state);

    // print conf to serial port
    Serial.println("Info:");
    Serial.println("Max Speed:" + String(speed_max) + "-->" + String(speed2rpm(speed_max, steps_p_rev, gear_ratio)) + " rpm");
    Serial.println("Min Speed:" + String(speed_min) + "-->" + String(speed2rpm(speed_min, steps_p_rev, gear_ratio)) + " rpm");
    Serial.println("Stepper Control Mode: \n Initialized");
}

void loop() {
    // state machine
    switch(state) {
        case 0:
            /* WAITING */
            desired_speed = map(analogRead(ANALOG_IN), 0, 1023, 0, speed_max);


            //if (timer_update_lcd.clock()) {
            //    long int start = millis();
//
            //    timer_update_lcd.reset();
            //    printState(lcd, desired_speed, stepper.speed(), steps_p_rev, state);
            //    long int stop = millis();
            //
            //    Serial.println(stop-start);
            //}

            if (desired_speed > (speed_min+hysterese)) {
                Serial.println("Starting");
                // enable stepper - current to coils
                stepper.enableOutputs();
                // indicate stepper is enabled with LED
                digitalWrite(LED_BUILTIN, HIGH);
                // switch state to running
                state   = 10;
            }
            break;
        case 10:
            /* RUNNING */
            while (desired_speed > speed_min)
            {   
                //long int start = millis();

                if (timer_update_speed.clock()) {

                    desired_speed = map(analogRead(ANALOG_IN), 0, 1023, 0, speed_max);
                    speed = stepper.speed();

                    timer_update_speed.reset();

                    if (abs(desired_speed - speed) > acceleration) {
                        if (desired_speed > speed) {
                            // Serial.println(String(desired_speed) + "-" + String(speed) + "->" + String(speed+acceleration));
                            stepper.setSpeed(speed+acceleration);
                        }
                        else if (desired_speed < speed) {
                            // Serial.println(String(desired_speed) + "-" + String(speed) + "->" + String(speed-acceleration));
                            stepper.setSpeed(speed-acceleration);
                        }
                    } else {
                        stepper.setSpeed(desired_speed);
                    }
                    // Serial.println("\r"+String(stepper.speed()));
                }
                
                if (timer_update_lcd.clock()) {
                    timer_update_lcd.reset();
                    Serial.println(String(speed2rpm(stepper.speed(), steps_p_rev, gear_ratio)));
                    //printState(lcd, desired_speed, stepper.speed(), steps_p_rev, state);
                }

                //long int stop = millis();
                //Serial.println(stop-start);

                stepper.runSpeed();
            }
            
            Serial.println("Stopping");
            // switch state to stopping
            state = 20;
            break;
        case 20:
            /* STOPPING */

            while (stepper.speed() > 0.0) {
                if (timer_update_speed.clock()) // call every 10 ms
                {
                    speed = stepper.speed();
                    timer_update_speed.reset();

                    if ((speed-acceleration) < 0)
                    {
                        stepper.setSpeed(0);
                    } else {
                        stepper.setSpeed(speed-acceleration);
                    }
                }

                stepper.runSpeed();

                //if (timer_update_lcd.clock())
                //{
                    //timer_update_lcd.reset();
                    //printState(lcd, desired_speed, stepper.speed(), steps_p_rev, state);
                //}
                
            }

            Serial.println("Stopped");
            Serial.println("Ready");
            // disable stepper
            stepper.disableOutputs();
            // indicate stepper has no current
            digitalWrite(LED_BUILTIN, LOW);
            // switch to initial/waiting state
            state = 0;

            break;
    }

}
