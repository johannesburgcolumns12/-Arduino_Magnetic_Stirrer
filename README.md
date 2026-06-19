# Arduino-Controlled Magnetic Stirrer with OLED Tachometer

## Overview
This project contains the source code and hardware documentation for a custom-built laboratory magnetic stirrer. Designed for precise chemical mixing and sample preparation, the system features an adjustable motor controller and an integrated OLED display that functions as a real-time tachometer to monitor rotations per minute (RPM).

## Features
* **Real-Time RPM Monitoring:** Utilizes a sensor via hardware interrupts to calculate and display live RPM on an I2C OLED screen.
* **Analog Speed Control:** Potentiometer-driven PWM signal for smooth motor acceleration and precise speed tuning.
* **Hardware-First Diagnostics:** Designed without diagnostic LEDs to minimize power draw and simplify the circuit. Voltage regulation and signal stability are verified using analog multimeter diagnostics.

## Components Used
* Arduino Uno
* I2C OLED Display (128x64 or 128x32)
* 12V DC Motor with Magnetic Stir Bar
* Logic-Level N-Channel MOSFET (or L298N Motor Driver)
* IR Sensor or Optical Interrupter (for RPM counting)
* 10kΩ Potentiometer

## Usage
1. Flash `magnetic_stirrer.ino` to the Arduino Uno.
2. Refer to `wiring_and_diagnostics.md` for pinout connections and hardware calibration.
3. Turn the potentiometer to adjust the stirring speed; the OLED will update the RPM reading every second.
