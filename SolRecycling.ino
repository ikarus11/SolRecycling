#include <AccelStepper.h>
#include <MultiStepper.h>
#include <LiquidCrystal.h>

/* --- CLASSES --- */
#include "Timer.h"

/* --- FUNCTIONS ---*/
#include "HelperFcn.h"

/* INPUTS */
// read potentiometer resistance
#define ANALOG_IN A7 // A0 for UNO A7 for NANO

/* --- STEPPER --- */
// Define the stepper motor connections
#define EN_PIN      4
#define STEP_PIN    5
#define DIR_PIN     6

// Create an AccelStepper object
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
// stepper variables
const int steps_p_rev   = 400;  // steps per revolution (max 150 rpm for 400 sps, gear ratio 1)
const float gear_ratio  = 1;    // driven / driving

/* --- TIMERS --- */
Timer timer_update_speed(100);    // read potetiometer every 100 ms
Timer timer_update_display(1000);     // lcd display update every 300 ms

/* VARIABLES */
// Vorgabe 120 rpm (limited to 1000 sps -> 150 rpm max if gear ratio = 1)
const int speed_max     = checkSpeed(rpm2speed(125, steps_p_rev, gear_ratio));
// Vorgabe 30 rpm
// stops if desired speed below
const int speed_min     = rpm2speed(20, steps_p_rev, gear_ratio);

// TODO: adjust acceleration value
// const float acceleration_SI = 5; // 1/s^2
// speed gets increased by acceleration every 100 ms
// e.g. acceleration = 5 leads to 1/40*acceleration*60 = 7.5 rpm/sec
// which results in 16 seconds to bring system from 0 to 120 rpm
const int acceleration      = 5;  // steps/s^2

/* STATE MACHINE */
int state           = 0;
int speed           = 0;
int desired_speed   = 0;

/* SIGNALS HANDLING */
// increase stability of loop
const int hysterese = 5;    // steps per second

/* MONITORING */
// float speed_rpm = speed2rpm(speed, steps_p_rev, gear_ratio);

/* SETUP / INITIALIZING */
void setup() {
    // Stepper configuration
    stepper.setMaxSpeed(speed_max);             // Set the maximum speed in steps per second
    stepper.setAcceleration(acceleration);      // Set the acceleration in steps per second per second    
    stepper.setCurrentPosition(0);              // Set the initial position
    stepper.setEnablePin(EN_PIN);               // Set en/disable pin of driver
    stepper.setPinsInverted(false,false,false); // DIRECTION/STEP/ENABLE e.g. change DIRESTION from false to true -> clw to ccw
    stepper.disableOutputs();                   // diable current to stepper
    
    // Set the initial states
    state           = 0;
    
    // enable serial communication
    Serial.begin(115200); // read output on this serial port
    
    // Timers
    timer_update_speed.start();
    timer_update_display.start();

    // LED configuration
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // print conf to serial port
    Serial.println("Info:");
    Serial.println("Max Speed:" + String(speed_max) + "-->" + String(speed2rpm(speed_max, steps_p_rev, gear_ratio)) + " rpm");
    Serial.println("Min Speed:" + String(speed_min) + "-->" + String(speed2rpm(speed_min, steps_p_rev, gear_ratio)) + " rpm");
    Serial.println("Stepper Control Mode: \n Initialized");
}

/* MAIN LOOP RUNNING FOREVER*/
void loop() {
    // state machine
    switch(state) {
        case 0:
            /* WAITING */
            desired_speed = map(analogRead(ANALOG_IN), 0, 1023, 0, speed_max);

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
                // update desired speed
                if (timer_update_speed.clock()) {
                    // read speed from potentiometer
                    desired_speed = map(analogRead(ANALOG_IN), 0, 1023, 0, speed_max);
                    speed = stepper.speed();

                    // reset timer
                    timer_update_speed.reset();
                    
                    // set new speed for stepper
                    if (abs(desired_speed - speed) > acceleration) {
                        if (desired_speed > speed) {
                            stepper.setSpeed(speed+acceleration);
                        }
                        else if (desired_speed < speed) {
                            stepper.setSpeed(speed-acceleration);
                        }
                    } else {
                        stepper.setSpeed(desired_speed);
                    }
                }
                
                // print rotaional speed to Serial
                if (timer_update_display.clock()) {
                    timer_update_display.reset();
                    Serial.println(String(speed2rpm(stepper.speed(), steps_p_rev, gear_ratio)));
                }

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

                if (timer_update_display.clock())
                {
                    timer_update_display.reset();
                    Serial.println(String(speed2rpm(stepper.speed(), steps_p_rev, gear_ratio)));
                }
                
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
