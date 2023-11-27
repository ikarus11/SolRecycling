#include <AccelStepper.h>
#include <MultiStepper.h>
#include <LiquidCrystal.h>

// Define the stepper motor connections
#define DIR_PIN     2
#define STEP_PIN    3
#define EN_PIN      4

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 11, en = 12, d4 = 5, d5 = 6, d6 = 7, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// functions
int rpm2speed(float rpm, int stepPerRev) {
    return rpm/60*stepPerRev;
}

float speed2rpm(int stepPerSec, int stepPerRev) {
    return (float)stepPerSec/stepPerRev*60.0;
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

void printState(int desired, int actual, int stepsPerRev, int stat){
    // Calculate desired and actual speeds
    float desiredSpeed = speed2rpm(desired, stepsPerRev);
    float actualSpeed = speed2rpm(actual, stepsPerRev);

    // Move the cursor to the beginning of the line and overwrite the content
    switch (stat)
    {
    case 0:
        lcd.setCursor(0, 0);
        lcd.print("SLEEP MODE");
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

// variables
const int steps_p_rev   = 200;                              // steps per revolution full step
const int speed_max     = rpm2speed(125, steps_p_rev);      // Vorgabe 120
const int speed_min     = rpm2speed(25, steps_p_rev);       // Vorgabe 30

// TODO: adjust values
const float acceleration_SI = 10;               // 1/s^2 
const int acceleration      = acceleration_SI;  // steps/s^2

int AnalogIn    = A0;       // Potentiometer

// states
int state           = 0;
int speed           = 0;
int desired_speed   = 0;

// signals and filters
const int hysterese = 5; // steps per second
const int resol     = 3; // resolution for potentiometer

// timeing
unsigned long previousMillis    = 0;
const long interval             = 100; // Interval of 1 second

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
    state           = 0;
    
    // enable communication
    Serial.begin(115200);
    
    // LED configuration
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    
    printState(desired_speed, speed, steps_p_rev, state);

    // print conf to serial port
    Serial.println("Info:");
    Serial.println("Max Speed:" + String(speed_max));
    Serial.println("Min Speed:" + String(speed_min));
    Serial.println("Stepper Control Mode:");
}

void loop() {
    // for debugging
    // Serial.println(speed);
    // lcd.noDisplay();
    // state machine
    switch(state) {
        case 0:
            /* WAITING */
            desired_speed = map(analogRead(AnalogIn), 0, 1023, 0, speed_max);

            printState(desired_speed, stepper.speed(), steps_p_rev, state);

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
            previousMillis = 0;

            while (desired_speed > speed_min)
            {
                desired_speed = map(analogRead(AnalogIn), 0, 1023, 0, speed_max);
                unsigned long currentMillis = millis();

                if (currentMillis - previousMillis >= interval) // call every 10 ms
                {
                    speed = stepper.speed();

                    previousMillis = currentMillis;

                    if (abs(desired_speed - speed) > acceleration) {
                        if (desired_speed > speed) {
                            // Serial.println(String(desired_speed) + "-" + String(speed) + "->" + String(speed+acceleration));
                            stepper.setSpeed(speed+acceleration);
                        }
                        else if (desired_speed < speed) {
                            // Serial.println(String(desired_speed) + "-" + String(speed) + "->" + String(speed-acceleration));
                            stepper.setSpeed(speed-acceleration);
                        }
                    }
                    Serial.println("\r"+String(stepper.speed()));
                    printState(desired_speed, stepper.speed(), steps_p_rev, state);
                }
                
                stepper.runSpeed();
            }
            
            Serial.println("Stopping");
            // switch state to stopping
            state = 20;
            break;
        case 20:
            /* STOPPING */
            previousMillis = 0;

            while (stepper.speed() > 0.0) {
                unsigned long currentMillis = millis();

                if (currentMillis - previousMillis >= interval) // call every 10 ms
                {
                    previousMillis = currentMillis;
                    speed = stepper.speed();

                    stepper.setSpeed(speed-acceleration);
                    
                    printState(desired_speed, stepper.speed(), steps_p_rev, state);
                }
                
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
