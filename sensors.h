/*
 * Sensor Functions for Disaster Recon UAV
 *
 * Contains functions for reading all sensors:
 * - DHT11 (Temperature & Humidity)
 * - MQ-2 (Gas/Smoke)
 * - Ultrasonic (Distance)
 * - MPU6050 (Pitch, Roll, Yaw) - Using Adafruit library
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "config.h" // For sensor pins and thresholds
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>

// Global sensor objects
DHT dht(DHT_PIN, DHTTYPE);
Adafruit_MPU6050 mpu; // Adafruit MPU6050 object

// Sensor data structure
struct SensorData {
  float temperature; // °C
  float humidity;    // %
  int gasLevel;      // 0-1023
  int distance;      // cm
  float pitch;       // degrees
  float roll;        // degrees
  float yaw;         // degrees
  bool valid;        // All sensors read successfully
};

// ==========================================
// SENSOR INITIALIZATION
// ==========================================

void initializeSensors() {
  // Initialize DHT11
  dht.begin();

  // Initialize MPU6050 with Adafruit library
  Wire.begin();

  if (!mpu.begin()) {
    Serial.println(F("Failed to find MPU6050 chip"));
  } else {
    Serial.println(F("MPU6050 Found!"));

    // Configure MPU6050
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    Serial.println(F("MPU6050 configured"));
  }

  // MQ-2 is analog, no initialization needed

  // Initialize Ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW); // Ensure trigger starts LOW
  Serial.println(F("Ultrasonic sensor initialized"));

  Serial.println(F("Sensors initialized"));
}

// ==========================================
// DHT11 - Temperature & Humidity
// ==========================================

float readTemperature() {
  float temp = dht.readTemperature();
  if (isnan(temp)) {
    Serial.println(F("DHT11: Temperature read error"));
    return -999; // Error value
  }
  return temp;
}

float readHumidity() {
  float humid = dht.readHumidity();
  if (isnan(humid)) {
    Serial.println(F("DHT11: Humidity read error"));
    return -999; // Error value
  }
  return humid;
}

// ==========================================
// MQ-2 - Gas/Smoke Sensor
// ==========================================

int readGasLevel() {
  int gasValue = analogRead(MQ2_PIN);
  // Gas sensor returns 0-1023
  return gasValue;
}

bool isHazardousGas(int gasLevel) { return gasLevel >= MQ2_THRESHOLD; }

// ==========================================
// Ultrasonic - Distance Sensor
// ==========================================

int readDistance() {
  // Send ultrasonic pulse (pins already configured in initializeSensors)
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout

  // Calculate distance (speed of sound: 343 m/s or 0.034 cm/µs)
  int distance = duration * 0.034 / 2;

  // Validate range - allow 0-400cm, return -1 only if clearly invalid
  if (duration == 0 || distance > MAX_DISTANCE) {
    Serial.println(F("Ultrasonic: Out of range"));
    return -1; // Error value
  }

  return distance;
}

// ==========================================
// MPU6050 - Orientation Sensor (Adafruit)
// ==========================================

// Global sensor event objects
sensors_event_t a, g, temp;

void updateMPU() { mpu.getEvent(&a, &g, &temp); }

float readPitch() {
  // Calculate pitch from accelerometer (tilt forward/backward)
  return atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
}

float readRoll() {
  // Calculate roll from accelerometer (tilt left/right)
  return atan2(-a.acceleration.x, a.acceleration.z) * 180.0 / PI;
}

float readYaw() {
  // Yaw from gyroscope Z-axis
  return g.gyro.z * 180.0 / PI;
}

// ==========================================
// READ ALL SENSORS
// ==========================================

SensorData readAllSensors() {
  SensorData data;

  // Read DHT11
  data.temperature = readTemperature();
  data.humidity = readHumidity();

  // Read MQ-2
  data.gasLevel = readGasLevel();

  // Read Ultrasonic
  data.distance = readDistance();

  // Read MPU6050
  updateMPU();
  data.pitch = readPitch();
  data.roll = readRoll();
  data.yaw = readYaw();

  // Check if critical sensors failed (only check for explicit error values,
  // allow zero) DHT can return 0.0 in dry conditions, ultrasonic can be -1 if
  // out of range
  data.valid = (data.temperature != -999) && (data.humidity != -999);
  // Note: Distance can be -1 (out of range) but that's not a critical error

  return data;
}

#endif // SENSORS_H
