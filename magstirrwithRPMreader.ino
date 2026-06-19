/*
  Arduino-Controlled Magnetic Stirrer with OLED Tachometer

  This code controls a 12V DC motor via PWM (pin D9) for stirring speed,
  reads a 10k potentiometer (pin A0) for user input, and utilizes
  hardware interrupts (pin D2) with an optical interrupter or IR sensor
  to calculate and display real-time RPM on an I2C OLED display.

  Hardware Connections:
  - OLED: VCC -> 5V, GND -> GND, SDA -> A4, SCL -> A5
  - Potentiometer: Outer pins -> 5V & GND, Wiper -> A0
  - Motor Driver (MOSFET Gate): Pin D9
  - RPM Sensor (Signal): Pin D2

  Dependencies:
  - Adafruit SSD1306 library
  - Adafruit GFX library
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display Settings
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin Definitions
const int potPin = A0;       // Analog pin for potentiometer wiper
const int motorPin = 9;      // PWM digital pin for MOSFET gate control
const int rpmSensorPin = 2;  // Digital pin with interrupt capability (D2)

// RPM Calculation Variables
volatile unsigned long pulseCount = 0; // Volatile as it's modified in ISR
unsigned int rpm = 0;
unsigned long lastRpmCalculationTime = 0;
const unsigned long rpmCalculationInterval = 1000; // Calculate RPM every 1000ms (1 second)

// Optional: Number of pulses per revolution of your sensor wheel
// If your sensor wheel has 1 slot, use 1. If 2 slots, use 2.
const int pulsesPerRevolution = 1;

// Interrupt Service Routine (ISR) - Called on sensor pulse
void countPulse() {
  pulseCount++;
}

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(9600);
  Serial.println(F("Stirrer Starting..."));

  // Initialize pins
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW); // Ensure motor is off initially
  pinMode(rpmSensorPin, INPUT_PULLUP); // Internal pull-up for the sensor signal

  // Attach interrupt to the RPM sensor pin on FALLING edge
  // (Assuming sensor pulls signal low when blocked/activated)
  attachInterrupt(digitalPinToInterrupt(rpmSensorPin), countPulse, FALLING);

  // Initialize the OLED display (default I2C address 0x3C)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Display initial status/splash screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println(F("MagStirrer v1.0"));
  display.setCursor(10, 35);
  display.println(F("Initializing..."));
  display.display();
  delay(2000); // Pause for 2 seconds
}

void loop() {
  // --- 1. Motor Speed Control ---
  // Read the potentiometer value (0 to 1023)
  int potValue = analogRead(potPin);

  // Map the pot value to PWM range (0 to 255)
  int motorPWM = map(potValue, 0, 1023, 0, 255);

  // Write PWM signal to the motor driver
  analogWrite(motorPin, motorPWM);


  // --- 2. RPM Calculation (every interval) ---
  unsigned long currentTime = millis();
  if (currentTime - lastRpmCalculationTime >= rpmCalculationInterval) {
    // Disable interrupts temporarily to read pulseCount safely
    detachInterrupt(digitalPinToInterrupt(rpmSensorPin));

    // Calculate RPM: (Pulses in interval / interval_ms) * 60,000 ms/min / pulses_per_rev
    if (pulseCount > 0) {
        rpm = (unsigned int)((pulseCount * 60000UL) / (pulsesPerRevolution * rpmCalculationInterval));
    } else {
        rpm = 0;
    }

    // Reset pulseCount and timer
    pulseCount = 0;
    lastRpmCalculationTime = currentTime;

    // Re-enable interrupts
    attachInterrupt(digitalPinToInterrupt(rpmSensorPin), countPulse, FALLING);

    // Debug output
    // Serial.print("PWM: "); Serial.print(motorPWM); Serial.print(" | RPM: "); Serial.println(rpm);
  }


  // --- 3. Update OLED Display ---
  display.clearDisplay();

  // Draw Header
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("STIRRER STATUS"));
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);

  // Display RPM (Larger font)
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print(F("RPM: "));
  if (rpm < 100) display.print(F(" ")); // Leading space for alignment
  if (rpm < 10)  display.print(F(" "));
  display.print(rpm);

  // Display Speed Setting (e.g., as percentage)
  display.setTextSize(1);
  int speedPercentage = map(motorPWM, 0, 255, 0, 100);
  display.setCursor(10, 50);
  display.print(F("Speed: "));
  display.print(speedPercentage);
  display.println(F("%"));

  display.display();

  // Small delay to prevent display flickering
  delay(50);
}
