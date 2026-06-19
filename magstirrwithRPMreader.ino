#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin Definitions
const int motorPin = 9;       // PWM pin to MOSFET Gate
const int potPin = A0;        // Potentiometer wiper
const int sensorPin = 2;      // Interrupt pin for RPM sensor

// Variables for RPM Calculation
volatile unsigned int pulseCount = 0;
unsigned int rpm = 0;
unsigned long lastMillis = 0;

// Interrupt Service Routine (ISR)
void countPulse() {
  pulseCount++;
}

void setup() {
  pinMode(motorPin, OUTPUT);
  pinMode(sensorPin, INPUT_PULLUP);
  
  // Attach interrupt to the sensor pin (triggering on the FALLING edge)
  attachInterrupt(digitalPinToInterrupt(sensorPin), countPulse, FALLING);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;); // Loop forever if display fails
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.print("Stirrer Initialized");
  display.display();
  delay(2000);
}

void loop() {
  // 1. Motor Speed Control
  int potValue = analogRead(potPin);
  int motorSpeed = map(potValue, 0, 1023, 0, 255);
  analogWrite(motorPin, motorSpeed);

  // 2. RPM Calculation (Update every 1000 ms)
  if (millis() - lastMillis >= 1000) {
    // Disable interrupts temporarily to calculate RPM safely
    detachInterrupt(digitalPinToInterrupt(sensorPin));
    
    // Calculate RPM: (Pulses per second) * 60
    // Adjust the divider if your sensor outputs multiple pulses per revolution
    rpm = pulseCount * 60; 
    
    pulseCount = 0;
    lastMillis = millis();
    
    // Re-enable interrupts
    attachInterrupt(digitalPinToInterrupt(sensorPin), countPulse, FALLING);

    // 3. Update OLED Display
    updateDisplay(rpm, motorSpeed);
  }
}

void updateDisplay(unsigned int currentRPM, int pwmValue) {
  display.clearDisplay();
  
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("RPM: ");
  display.print(currentRPM);
  
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print("PWM Output: ");
  // Convert 0-255 PWM to a rough percentage
  display.print(map(pwmValue, 0, 255, 0, 100)); 
  display.print("%");
  
  display.display();
}
