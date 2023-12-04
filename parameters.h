// In this file are defined all important control parameters
// steps per revolution (max 150 rpm for 400 sps, gear ratio 1)
const int   STEPS_P_REV = 400; // see motor driver for adjustments
// direction of stepper
const bool  CW = false; // (true or false)
// driven / driving
const float GEAR = 1;
// read potentiometer input every e.g. 100 ms
const int   UPDATE_POT = 100;
// print speed every e.g. 1000 ms to serial
const int   UPDATE_DISP = 1000;
// stops if desired speed below
const int   RPM_MIN = 20;
// Vorgabe 120 rpm (limited to 1000 sps -> 150 rpm max if gear ratio = 1)
const int   RPM_MAX = 130; 
// speed gets increased by acceleration every 100 ms
// e.g. acceleration = 5 leads to 1/40*acceleration*60 = 7.5 rpm/sec
// which results in 16 seconds to bring system from 0 to 120 rpm
const int   ACCELERATION = 5;  // steps/s^2
