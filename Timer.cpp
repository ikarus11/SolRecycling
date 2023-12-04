// Timer.cpp - Implementation file for Timer class

#include "Timer.h"

Timer::Timer(unsigned long clockTarget) {
  _running       = false;
  _targetTime    = clockTarget;
  _startTime     = 0;
  _elapsedTime   = 0;
}

void Timer::start() {
  if (!_running) {
    _startTime   = millis(); // Store the current time
    _running     = true;
  }
}

void Timer::stop() {
  if (_running) {
    _elapsedTime = millis() - _startTime; // Update elapsed time
    _running     = false;
  }
}

void Timer::reset(){
    _elapsedTime = 0;
    _running     = true;
    _startTime   = millis();
}

bool Timer::clock(){
    _elapsedTime = millis() - _startTime;
    if (_elapsedTime >= _targetTime) {
        return true;
    }
    else {
        return false;
    }
}

unsigned long Timer::getTime() {
  if (_running) {
    _elapsedTime = millis() - _startTime; 
    return _elapsedTime;
  } else {
    return _elapsedTime;
  }
}
