/*
 * Configuration File for Disaster Recon UAV
 *
 * This file contains all pin definitions, sensor thresholds,
 * and constant values used throughout the project.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// PIN DEFINITIONS
// ==========================================

// Sensors
#define DHT_PIN 12  // DHT11 Temperature & Humidity sensor (D12)
#define TRIG_PIN 10 // Ultrasonic sensor - Trigger (D10)
#define ECHO_PIN 11 // Ultrasonic sensor - Echo (D11)
#define IR_PIN 2    // IR Receiver for remote control (D2)
#define MQ2_PIN A0  // MQ-2 Gas/Smoke sensor (analog A0)

// Actuators
#define BUZZER_PIN 9 // Piezo Buzzer (D9 PWM)
#define RGB_RED 6    // RGB LED - Red channel (D6 PWM)
#define RGB_GREEN 7  // RGB LED - Green channel (D7 PWM)
#define RGB_BLUE 8   // RGB LED - Blue channel (D8 PWM)

// DC Motor Control (L293D)
#define MOTOR_IN1 4    // L293D IN1 (D4 - direction control)
#define MOTOR_IN2 5    // L293D IN2 (D5 - direction control)
#define MOTOR_ENABLE 3 // L293D Enable (D3 PWM - speed control)

// I2C Devices (use hardware I2C pins)
#define LCD_ADDRESS 0x27 // LCD I2C address
#define MPU_ADDRESS 0x68 // MPU6050 I2C address

// ==========================================
// SENSOR THRESHOLDS
// ==========================================

// Gas/Smoke Detection
#define MQ2_THRESHOLD 460 // 45% of 1023 (analog scale)

// Distance Limits (cm)
#define MIN_DISTANCE 2   // Minimum valid distance
#define MAX_DISTANCE 400 // Maximum valid distance

// Temperature Limits (°C) - optional warnings
#define TEMP_WARNING_HIGH 40
#define TEMP_WARNING_LOW 0

// ==========================================
// DRONE STATE DEFINITIONS
// ==========================================

#define STATE_OFF 0    // UAV powered off
#define STATE_IDLE 1   // UAV on, initializing
#define STATE_ACTIVE 2 // Normal operation, collecting data
#define STATE_ALERT 3  // Hazard detected (high gas)
#define STATE_ERROR 4  // Sensor malfunction

// ==========================================
// COMMUNICATION SETTINGS
// ==========================================

#define BAUD_RATE 9600        // Serial communication baud rate
#define UPDATE_INTERVAL 15000 // Data update interval (15 seconds in ms)

// ==========================================
// IR REMOTE CODES
// ==========================================
// Scanned from actual remote - 2025-11-29

#define IR_BUTTON_ON 0x47E9055D  // Button "1" - Turn UAV ON
#define IR_BUTTON_OFF 0x3BDAB6CD // Button "2" - Turn UAV OFF

// ==========================================
// BUZZER TONES & DURATIONS
// ==========================================

#define TONE_READY 1000  // Hz - Ready beep
#define TONE_HAZARD 2000 // Hz - Hazard alert
#define TONE_STATE 1500  // Hz - State change

#define BEEP_SHORT 100  // ms - Short beep
#define BEEP_MEDIUM 200 // ms - Medium beep
#define BEEP_GAP 300    // ms - Gap between beeps

// ==========================================
// DHT SENSOR TYPE
// ==========================================

#define DHTTYPE DHT11

// ==========================================
// LCD DISPLAY SETTINGS
// ==========================================

#define LCD_COLS 16 // LCD columns
#define LCD_ROWS 2  // LCD rows

#endif // CONFIG_H
