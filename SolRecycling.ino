#include <AccelStepper.h>

// Define the stepper motor connections
#define DIR_PIN     2
#define STEP_PIN    3
#define EN_PIN      4

// functions
int rpm2speed(float rpm, int stepPerRev) {
    return rpm/60*stepPerRev;
}

float speed2rpm(int stepPerSec, int stepPerRev) {
    return stepPerSec/stepPerRev*60;
}

bool detectChange(int currentSpeed, int previousSpeed, int treashold) {
    if (abs(previousSpeed - currentSpeed) > treashold) {
        return true;
    }
    else
    {
        return false;
    }
}

// variables
const int steps_p_rev   = 200;                              // steps per revolution full step
const int speed_max     = rpm2speed(125, steps_p_rev);      // Vorgabe 120
const int speed_min     = rpm2speed(25, steps_p_rev);       // Vorgabe 30

// TODO: adjust values
const float acceleration_SI = 1;               // 1/s^2 
const int acceleration      = acceleration_SI;  // steps/s^2

int AnalogIn    = A0;       // Potentiometer

// states
int state       = 0;
int speed       = 0;
int speed_old   = 0;

// signals and filters
const int hysterese = 5; // steps per second
const int resol     = 3; // resolution for potentiometer

// SI
float speed_rpm     = speed2rpm(speed, steps_p_rev);

// Create an AccelStepper object
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

void setup() {
    // Stepper configuration
    stepper.setMaxSpeed(speed_max);         // Set the maximum speed in steps per second
    stepper.setAcceleration(acceleration);  // Set the acceleration in steps per second per second    
    stepper.setCurrentPosition(0);          // Set the initial position
    stepper.setEnablePin(EN_PIN);           // Set en/disable pin of driver
    stepper.setPinsInverted(false,false,true);
    stepper.disableOutputs();               // diable current to stepper
    
    // Set the initial states
    state       = 0;
    speed_old   = 0;
    
    // enable communication
    Serial.begin(115200);
    
    // LED configuration
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // print conf to serial port
    Serial.println("Info:");
    Serial.println("Max Speed:" + String(speed_max));
    Serial.println("Min Speed:" + String(speed_min));
    Serial.println("Stepper Control Mode:");
}

void loop() {
    // for debugging
    // Serial.println(speed);

    // state machine
    switch(state) {
        case 0:
            /* WAITING */
            speed = map(analogRead(AnalogIn), 0, 1023, 0, speed_max);

            if (speed > (speed_min+hysterese)) {
                Serial.println("Starting");
                // enable stepper - current to coils
                stepper.enableOutputs();
                stepper.setSpeed(speed);
                // indicate stepper is enabled with LED
                digitalWrite(LED_BUILTIN, HIGH);
                // switch state to running
                state   = 10;
            }
            break;
        case 10:
            /* RUNNING */
            while (speed > speed_min)
            {
                speed = map(analogRead(AnalogIn), 0, 1023, 0, speed_max);

                if (detectChange(speed, speed_old, resol))
                {
                    //Serial.println(speed);
                    stepper.setSpeed(speed);
                    speed_old = speed;
                }

                stepper.runSpeed();
                // Serial.println(String(speed) + "-" + String(stepper.speed()));
            }
            
            Serial.println("Stopping");
            // switch state to stopping
            state = 20;
            break;
        case 20:
            /* STOPPING */
            stepper.setSpeed(0);

            while (stepper.speed()!=0)
            {
                stepper.runSpeed();
            }
            
            Serial.println("Stopped");
            // disable stepper
            stepper.disableOutputs();
            // indicate stepper has no current
            digitalWrite(LED_BUILTIN, LOW);
            // switch to initial/waiting state
            state = 0;

            break;
    }

}
