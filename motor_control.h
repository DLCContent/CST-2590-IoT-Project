/*
 * Motor Control & State Machine for Disaster Recon UAV
 *
 * Handles:
 * - DC Motor control (ON/OFF)
 * - UAV state management
 * - State transitions
 *
 * Motor Driver: L293D H-Bridge
 * Wiring Instructions:
 * - Connect motor to OUT1/OUT2 on L293D
 * - Connect MOTOR_ENABLE (D3) to Enable1 on L293D
 * - Connect MOTOR_IN1 (D7) to IN1 on L293D
 * - Connect MOTOR_IN2 (D8) to IN2 on L293D
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "config.h"  // For motor pins and state constants
#include "sensors.h" // For SensorData type

// ==========================================
// MOTOR CONTROL
// ==========================================

void initializeMotor() {
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_ENABLE, OUTPUT);

  // Start with motor OFF
  analogWrite(MOTOR_ENABLE, 0); // Speed = 0
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
}

void motorOn() {
  digitalWrite(MOTOR_IN1, HIGH);  // Forward direction
  digitalWrite(MOTOR_IN2, LOW);   // Forward direction
  analogWrite(MOTOR_ENABLE, 255); // Full speed (PWM)
  Serial.println(F("Motor: ON (Full Speed)"));
}

void motorOff() {
  analogWrite(MOTOR_ENABLE, 0); // Speed = 0 (stop)
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  Serial.println(F("Motor: OFF"));
}

// ==========================================
// STATE MACHINE
// ==========================================

int currentState = STATE_OFF;
int previousState = STATE_OFF;

int getCurrentState() { return currentState; }

void setState(int newState) {
  if (newState != currentState) {
    previousState = currentState;
    currentState = newState;

    // Beep on state change
    beepStateChange();

    // Print state change to Serial
    Serial.print(F("State changed: "));
    Serial.print(previousState);
    Serial.print(F(" -> "));
    Serial.println(currentState);

    // Handle state-specific actions
    switch (currentState) {
    case STATE_OFF:
      motorOff();
      rgbOff();
      displayOff();
      break;

    case STATE_IDLE:
      motorOn();
      rgbPurple();
      lcd.backlight(); // Ensure backlight is on
      lcd.clear();
      lcd.print(F("UAV ACTIVE"));
      lcd.setCursor(0, 1);
      lcd.print(F("Warming up..."));
      break;

    case STATE_ACTIVE:
      motorOn();
      rgbGreen();
      lcd.backlight(); // Ensure backlight is on
      break;

    case STATE_ALERT:
      motorOn();
      rgbRed();
      displayAlert();
      beepHazard(); // 3 beeps
      break;

    case STATE_ERROR:
      motorOff();
      rgbYellow();
      displayError();
      break;
    }
  }
}

// Update state based on sensor readings
void updateState(SensorData data) {
  // Only update state if UAV is ON (not in OFF state)
  if (currentState == STATE_OFF) {
    return; // Don't auto-transition from OFF
  }

  // DISABLED: Error checking - motor wiring issue
  // if (!data.valid) {
  //   setState(STATE_ERROR);
  //   return;
  // }

  // Check for hazards
  if (isHazardousGas(data.gasLevel)) {
    setState(STATE_ALERT);
    return;
  }

  // Normal operation
  if (currentState == STATE_IDLE || currentState == STATE_ACTIVE ||
      currentState == STATE_ALERT) {
    // Return to ACTIVE if no hazards
    if (currentState != STATE_ACTIVE) {
      setState(STATE_ACTIVE);
    }
  }
}

#endif // MOTOR_CONTROL_H
