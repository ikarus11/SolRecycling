# SolRecycling

## Infos
- Microcontroller: Arduino Nano
- Stepper Driver: DM860H 2H Microstep Driver 24-60 VDC
- Stepper Motor: NEMA 23 - 4.2A - 1.8°
- Power Supply: 48VDC - 12.5A
- Green LED Indicates Stepper is operating (bestromt)

## Warnings
- Don't power the stepper driver without the arduino connected!
- Currently there's no safety switch!
- Protect arduino from damge trough collisions or dust
- Make sure cabels can not be accidently disconnected!
- Properly isolate cabels and power supply!

## Configuration
Control parameters can be easily adjusted in parameters.h file. (Pay attention to physical/electical limits!)

## ToDo's
- Add safety switch directly to the power supply (manually cut off current).
- Replace Motor (> 20Nm)
- Adjust motor driver current to fit the new motor
