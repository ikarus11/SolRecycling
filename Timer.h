// Timer.h - Header file for Timer class

#ifndef Timer_H
#define Timer_H

#include <Arduino.h>

class Timer {
  public:
    Timer(unsigned long clockTarget = 100);                    // Constructor
    void start();               // Start the timer
    void stop();                // Stop the timer
    void reset();               // Reset the timer
    unsigned long getTime();    // Get the elapsed time in milliseconds
    bool clock();

  private:
    bool _running;
    unsigned long _targetTime;
    unsigned long _startTime;
    unsigned long _elapsedTime;
};

#endif
