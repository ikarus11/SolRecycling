#include <AccelStepper.h>
#include <MultiStepper.h>
#include <LiquidCrystal.h>

/* --- CLASSES --- */
#include "Timer.h"

/* --- FUNCTIONS ---*/
#include "HelperFcn.h"

/* --- PARAMETERS ---*/
#include "parameters.h"

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

/* --- TIMERS --- */
Timer timer_update_speed(UPDATE_POT);
Timer timer_update_display(UPDATE_DISP);

/* --- VARIABLES ---*/
const int speed_max     = checkSpeed(rpm2speed(RPM_MIN, STEPS_P_REV, GEAR));
const int speed_min     = rpm2speed(RPM_MAX, STEPS_P_REV, GEAR);

// STATE MACHINE
int state           = 0;
int speed           = 0;
int desired_speed   = 0;

/*--- SIGNALS HANDLING ---*/
// increase stability of loop
const int hysterese = 5;    // steps per second

/*--- MONITORING ---*/
// float speed_rpm = speed2rpm(speed, STEPS_P_REV, GEAR);

/*--- SETUP / INITIALIZING ---*/
void setup() {
    // Stepper configuration
    stepper.setMaxSpeed(speed_max);                 // Set the maximum speed in steps per second
    stepper.setAcceleration(ACCELERATION);          // Set the acceleration in steps per second per second    
    stepper.setCurrentPosition(0);                  // Set the initial position
    stepper.setEnablePin(EN_PIN);                   // Set en/disable pin of driver
    stepper.setPinsInverted(CW,false,false); // DIRECTION/STEP/ENABLE e.g. change DIRESTION from false to true -> clw to ccw
    stepper.disableOutputs();                       // diable current to stepper
    
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
    Serial.println("Max Speed:" + String(speed_max) + "-->" + String(speed2rpm(speed_max, STEPS_P_REV, GEAR)) + " rpm");
    Serial.println("Min Speed:" + String(speed_min) + "-->" + String(speed2rpm(speed_min, STEPS_P_REV, GEAR)) + " rpm");
    Serial.println("Stepper Control Mode: \n Initialized");
}

/*--- MAIN LOOP RUNNING FOREVER ---*/
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
                    if (abs(desired_speed - speed) > ACCELERATION) {
                        if (desired_speed > speed) {
                            stepper.setSpeed(speed+ACCELERATION);
                        }
                        else if (desired_speed < speed) {
                            stepper.setSpeed(speed-ACCELERATION);
                        }
                    } else {
                        stepper.setSpeed(desired_speed);
                    }
                }
                
                // print rotaional speed to Serial
                if (timer_update_display.clock()) {
                    timer_update_display.reset();
                    Serial.println(String(speed2rpm(stepper.speed(), STEPS_P_REV, GEAR)));
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

                    if ((speed-ACCELERATION) < 0)
                    {
                        stepper.setSpeed(0);
                    } else {
                        stepper.setSpeed(speed-ACCELERATION);
                    }
                }

                stepper.runSpeed();

                if (timer_update_display.clock())
                {
                    timer_update_display.reset();
                    Serial.println(String(speed2rpm(stepper.speed(), STEPS_P_REV, GEAR)));
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
