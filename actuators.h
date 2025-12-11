/*
 * Actuator Functions for Disaster Recon UAV
 *
 * Controls all output devices:
 * - LCD Display (I2C 16x2)
 * - RGB LED (status indicator)
 * - Piezo Buzzer (alerts)
 */

#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "config.h"  // For LCD_ADDRESS, RGB pins, buzzer constants
#include "sensors.h" // For SensorData type
#include <LiquidCrystal_I2C.h>

// Global LCD object
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// ==========================================
// LCD INITIALIZATION
// ==========================================

void initializeLCD() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print(F("UAV Initializing"));
  lcd.setCursor(0, 1);
  lcd.print(F("Please wait..."));
}

// ==========================================
// LCD DISPLAY FUNCTIONS
// ==========================================

void displayReady() {
  lcd.backlight();
  lcd.clear();
  lcd.print(F("Ready to Fly!"));
  lcd.setCursor(0, 1);
  lcd.print(F("Press IR ON"));
}

void displayOff() {
  lcd.backlight();
  lcd.clear();
  lcd.print(F("UAV OFF"));
  lcd.setCursor(0, 1);
  lcd.print(F("Standby Mode"));
}

void displaySensorData(SensorData data, int displayMode) {
  lcd.backlight();
  lcd.clear();

  // Rotate display between different sensor readings
  switch (displayMode % 4) {
  case 0: // Temperature & Humidity
    lcd.print(F("T:"));
    lcd.print(data.temperature, 1);
    lcd.print(F("C H:"));
    lcd.print(data.humidity, 0);
    lcd.print(F("%"));
    lcd.setCursor(0, 1);
    lcd.print(F("Gas:"));
    lcd.print(data.gasLevel);
    break;

  case 1: // Gas & Distance
    lcd.print(F("Gas:"));
    lcd.print(data.gasLevel);
    lcd.print(F(" ("));
    lcd.print((data.gasLevel * 100) / 1023);
    lcd.print(F("%)"));
    lcd.setCursor(0, 1);
    lcd.print(F("Dist:"));
    lcd.print(data.distance);
    lcd.print(F(" cm"));
    break;

  case 2: // Pitch & Roll
    lcd.print(F("Pitch:"));
    lcd.print(data.pitch, 1);
    lcd.setCursor(0, 1);
    lcd.print(F("Roll:"));
    lcd.print(data.roll, 1);
    break;

  case 3: // Yaw
    lcd.print(F("Yaw:"));
    lcd.print(data.yaw, 1);
    lcd.setCursor(0, 1);
    lcd.print(F("Heading"));
    break;
  }
}

void displayAlert() {
  lcd.backlight();
  lcd.clear();
  lcd.print(F("!!! ALERT !!!"));
  lcd.setCursor(0, 1);
  lcd.print(F("GAS DETECTED!"));
}

void displayError() {
  lcd.backlight();
  lcd.clear();
  lcd.print(F("SENSOR ERROR"));
  lcd.setCursor(0, 1);
  lcd.print(F("Check wiring"));
}

// ==========================================
// RGB LED FUNCTIONS
// ====================================================================================

void setRGBColor(int red, int green, int blue) {
  analogWrite(RGB_RED, red);
  analogWrite(RGB_GREEN, green);
  analogWrite(RGB_BLUE, blue);
}

void initializeRGB() {
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);
  setRGBColor(0, 0, 0); // Start with LED off
}

// Status colors
void rgbOff() { setRGBColor(0, 0, 0); }

void rgbGreen() { // Normal operation
  setRGBColor(0, 255, 0);
}

void rgbBlue() { // Uploading data
  setRGBColor(0, 0, 255);
}

void rgbRed() { // Hazard alert
  setRGBColor(255, 0, 0);
}

void rgbYellow() { // Warning/Gas
  setRGBColor(255, 255, 0);
}

void rgbPurple() { // Idle/ready
  setRGBColor(128, 0, 128);
}

// Environmental alert colors
void rgbOrange() { // Fire alert (≥30°C)
  setRGBColor(255, 165, 0);
}

void rgbCyan() { // Cold/blizzard alert (≤20°C)
  setRGBColor(0, 255, 255);
}

void rgbDeepBlue() { // Hurricane alert (99-120% humidity)
  setRGBColor(0, 0, 139);
}

// ==========================================
// PIEZO BUZZER FUNCTIONS
// ==========================================

void initializeBuzzer() { pinMode(BUZZER_PIN, OUTPUT); }

// Short beep - for ready and IR press
void beepShort() {
  tone(BUZZER_PIN, TONE_READY, 100);
  delay(150);
  noTone(BUZZER_PIN);
}

// Ready beep (same as short beep)
void beepReady() { beepShort(); }

// IR button press beep
void beepIR() { beepShort(); }

// State change beep (keep for compatibility)
void beepStateChange() { beepShort(); }

// Obstacle/Tilt - short bursts (5 quick beeps)
void beepObstacle() {
  for (int i = 0; i < 5; i++) {
    tone(BUZZER_PIN, TONE_HAZARD, 80);
    delay(100);
    noTone(BUZZER_PIN);
    delay(50);
  }
}

// Environmental hazard - medium length beeps (Beeep Beeep Beeep)
void beepEnvironmental() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, TONE_HAZARD, 300);
    delay(400);
    noTone(BUZZER_PIN);
    delay(200);
  }
}

// Gas hazard (use environmental pattern)
void beepHazard() { beepEnvironmental(); }

#endif // ACTUATORS_H
