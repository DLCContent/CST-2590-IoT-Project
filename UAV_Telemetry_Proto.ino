/*
 * =====================================================
 * DISASTER RECON UAV - Main Telemetry Sketch
 * =====================================================
 *
 * This sketch integrates all UAV components:
 * - Sensors: DHT11, MQ-2, Ultrasonic, MPU6050
 * - Actuators: LCD, RGB LED, Piezo Buzzer, DC Motor
 * - Control: IR Remote
 * - Communication: Serial to ThingSpeak via Python
 *
 * Date: 2025-11-30
 * =====================================================
 */

#include <Adafruit_MPU6050.h> // Adafruit MPU6050 library
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <IRremote.hpp>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>


// Include our modular header files
#include "actuators.h"
#include "config.h"
#include "motor_control.h"
#include "sensors.h"

// Cloud integration variables
struct ThingSpeakData {
  float temp; float humid; int gas; int dist; 
  int state; float pitch; float roll; float yaw;
};
ThingSpeakData receivedTSData;
bool cloudConnected = false;
#define THINGSPEAK_MODE 5


// ==========================================
// GLOBAL VARIABLES
// ==========================================

unsigned long lastUpdateTime = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastDHTUpdate = 0; // Separate timer for DHT sensor
int displayMode = 0;

SensorData currentData;

// ==========================================
// SETUP
// ==========================================

void setup() {
  // Initialize Serial communication
  Serial.begin(BAUD_RATE);
  Serial.println(F("=============================="));
  Serial.println(F(" Disaster Recon UAV Starting"));
  Serial.println(F("=============================="));

  // Initialize LCD first for user feedback
  initializeLCD();
  delay(2000);

  // Initialize all components
  Serial.println(F("Initializing sensors..."));
  initializeSensors();

  Serial.println(F("Initializing actuators..."));
  initializeRGB();
  initializeBuzzer();

  Serial.println(F("Initializing motor..."));
  initializeMotor();

  Serial.println(F("Initializing IR receiver..."));
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK); // Start IR receiver

  // Ready signal
  beepReady(); // Short beep when ready
  displayReady();
  rgbPurple();

  Serial.println(F("System ready! Waiting for IR ON command..."));
  Serial.println(F(""));
}

// ==========================================
// MAIN LOOP
// ==========================================

unsigned long loopCount = 0; // Debug counter

void loop() {
  // Debug: Print every 100 loops to confirm loop is running
  loopCount++;
  if (loopCount % 100 == 0) {
    Serial.print(F("Loop running... count: "));
    Serial.println(loopCount);
  }

  // Check for IR remote commands
  handleIRRemote();

  // If UAV is ON, collect and transmit data
  if (currentState != STATE_OFF) {
    // Check if it's time to update sensors
    unsigned long currentTime = millis();
    
    // Check for cloud data if connected
    if (cloudConnected) {
      readThingSpeakData();
    }
    // Update DHT sensor more frequently (every 2 seconds for real-time display)
    if (currentTime - lastDHTUpdate >= 2000) {
      lastDHTUpdate = currentTime;
      currentData.temperature = readTemperature();
      currentData.humidity = readHumidity();

      // Update distance and MPU every 2 seconds for obstacle detection
      currentData.distance = readDistance();
      updateMPU();
      currentData.pitch = readPitch();
      currentData.roll = readRoll();
      currentData.yaw = readYaw();

      // ========================================
      // ENVIRONMENTAL THRESHOLD DETECTION
      // ========================================

      // Priority 1: Check for FIRE (Temperature ≥ 30°C)
      if (currentData.temperature >= 30) {
        lcd.clear();
        lcd.print(F("FIRE DETECTED!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Temp: "));
        lcd.print(currentData.temperature, 1);
        lcd.print(F("C"));
        beepEnvironmental(); // Beeep Beeep Beeep
        rgbOrange();
        delay(100);
        rgbOff();
      }
      // Priority 2: Check for COLD/BLIZZARD (Temperature ≤ 20°C)
      else if (currentData.temperature <= 20) {
        lcd.clear();
        lcd.print(F("BLIZZARD!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Temp: "));
        lcd.print(currentData.temperature, 1);
        lcd.print(F("C"));
        beepEnvironmental(); // Beeep Beeep Beeep
        rgbCyan();
        delay(100);
        rgbOff();
      }
      // Priority 3: Check for HURRICANE (Humidity 99-120%)
      else if (currentData.humidity >= 99 && currentData.humidity <= 120) {
        lcd.clear();
        lcd.print(F("HURRICANE!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Humid: "));
        lcd.print(currentData.humidity, 0);
        lcd.print(F("%"));
        beepEnvironmental(); // Beeep Beeep Beeep
        rgbDeepBlue();
        delay(100);
        rgbOff();
      }
      // Priority 4: Check for GAS
      else if (isHazardousGas(currentData.gasLevel)) {
        lcd.clear();
        lcd.print(F("GAS DETECTED!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Level: "));
        lcd.print(currentData.gasLevel);
        beepEnvironmental(); // Beeep Beeep Beeep
        rgbYellow();
        delay(100);
        rgbOff();
      }
      // Priority 5: Check for obstacles (distance < 5cm)
      else if (currentData.distance > 0 && currentData.distance < 5) {
        lcd.clear();
        lcd.print(F("OBSTACLE!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Pulling up..."));
        beepObstacle(); // 5 short bursts
        rgbRed();
        delay(50);
        rgbOff();
      }
      // Priority 6: Check for excessive tilt (MPU)
      else if (abs(currentData.pitch) > 45 || abs(currentData.roll) > 45) {
        lcd.clear();
        lcd.print(F("TILT ALERT!"));
        lcd.setCursor(0, 1);
        lcd.print(F("Leveling..."));
        beepObstacle(); // 5 short bursts
        rgbRed();
        delay(50);
        rgbOff();
      }
    }

    // Update sensors and send data every UPDATE_INTERVAL (15 seconds)
    if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
      lastUpdateTime = currentTime;

      // Read gas sensor
      currentData.gasLevel = readGasLevel();

      // Update state based on sensor readings
      updateState(currentData);

      // Send data to Serial (for Python to upload to ThingSpeak)
      sendDataToSerial(currentData);

      // Brief flash to indicate data transmission
      rgbBlue();
      delay(200);

      // Restore state color
      if (currentState == STATE_ACTIVE)
        rgbGreen();
      else if (currentState == STATE_ALERT)
        rgbRed();
    }

    // Update LCD display (rotate every 10 seconds)
    if (currentTime - lastDisplayUpdate >= 10000) {
      lastDisplayUpdate = currentTime;

      if (currentState == STATE_ACTIVE) {
        displayMode++;
        if (displayMode > THINGSPEAK_MODE) {
          displayMode = 0;
        }
        
        if (displayMode == THINGSPEAK_MODE && cloudConnected) {
          displayThingSpeakData(0);
        } else {
          displaySensorData(currentData, displayMode);
        }
      }
    }
  }

  // Small delay for stability
  delay(50);
}

// ==========================================
// IR REMOTE HANDLER
// ==========================================

void handleIRRemote() {
  // Check if IR data is available
  if (IrReceiver.decode()) {
    unsigned long irCode = IrReceiver.decodedIRData.decodedRawData;

    // Ignore repeat codes (0x0) - only process actual button codes
    if (irCode != 0x0) {
      Serial.println(F("=== IR SIGNAL DETECTED ==="));
      Serial.print(F("IR Code received: 0x"));
      Serial.println(irCode, HEX);

      beepIR(); // Short beep for IR button press

      // SIMPLIFIED: ANY button toggles UAV ON/OFF
      if (currentState == STATE_OFF) {
        // Turn ON
        Serial.println(F("IR: Turning UAV ON"));
        setState(STATE_IDLE);
        delay(1000); // Brief warm-up period
        setState(STATE_ACTIVE);
        
        // Connect to cloud
        connectToCloud();
      } else {
        // Turn OFF
        Serial.println(F("IR: Turning UAV OFF"));
        cloudConnected = false;
        setState(STATE_OFF);
      }
    }

    IrReceiver.resume(); // Enable receiving of the next value
  }
}

// ==========================================
// SEND DATA TO SERIAL (CSV FORMAT)
// ==========================================

void sendDataToSerial(SensorData data) {
  // Send CSV formatted data for Python script
  // Format: temp,humid,gas,dist,state,pitch,roll,yaw

  Serial.print(data.temperature, 1);
  Serial.print(F(","));
  Serial.print(data.humidity, 1);
  Serial.print(F(","));
  Serial.print(data.gasLevel);
  Serial.print(F(","));
  Serial.print(data.distance);
  Serial.print(F(","));
  Serial.print(currentState);
  Serial.print(F(","));
  Serial.print(data.pitch, 1);
  Serial.print(F(","));
  Serial.print(data.roll, 1);
  Serial.print(F(","));
  Serial.println(data.yaw, 1); // println for newline at end
}


// Cloud connection function
void connectToCloud() {
  lcd.clear();
  lcd.print(F("Connecting to"));
  lcd.setCursor(0, 1);
  lcd.print(F("cloud..."));
  Serial.println("CONNECT_CLOUD");
  
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {
    if (Serial.available()) {
      String response = Serial.readStringUntil('\n');
      if (response == "CLOUD_OK") {
        cloudConnected = true;
        lcd.clear();
        lcd.print(F("Connected!"));
        beepReady();
        delay(2000);
        return;
      } else if (response == "CLOUD_FAIL") {
        cloudConnected = false;
        lcd.clear();
        lcd.print(F("Connection"));
        lcd.setCursor(0, 1);
        lcd.print(F("failed!"));
        delay(2000);
        return;
      }
    }
    delay(100);
  }
  cloudConnected = false;
  lcd.clear();
  lcd.print(F("Timeout!"));
  delay(2000);
}

// Read ThingSpeak data from serial
void readThingSpeakData() {
  static String receivedString = "";
  static boolean newData = false;
  
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n') {
      newData = true;
      break;
    }
    if (isDigit(inChar) || inChar == '.' || inChar == '-' || inChar == '|') {
      receivedString += inChar;
    }
  }
  
  if (newData) {
    char tempStr[receivedString.length() + 1];
    receivedString.toCharArray(tempStr, sizeof(tempStr));
    char *token = strtok(tempStr, "|");
    int fieldIndex = 0;
    
    while (token != NULL) {
      float value = atof(token);
      switch (fieldIndex) {
        case 0: receivedTSData.temp = value; break;
        case 1: receivedTSData.humid = value; break;
        case 2: receivedTSData.gas = (int)value; break;
        case 3: receivedTSData.dist = (int)value; break;
        case 4: receivedTSData.state = (int)value; break;
        case 5: receivedTSData.pitch = value; break;
        case 6: receivedTSData.roll = value; break;
        case 7: receivedTSData.yaw = value; break;
      }
      fieldIndex++;
      token = strtok(NULL, "|");
    }
    receivedString = "";
    newData = false;
  }
}

// Display ThingSpeak cloud data
void displayThingSpeakData(int mode) {
  static int tsDisplayStep = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if (tsDisplayStep == 0) {
    lcd.print(F("Cloud T/H: "));
    lcd.print(receivedTSData.temp, 1);
    lcd.setCursor(0, 1);
    lcd.print(receivedTSData.humid, 0);
    lcd.print(F("%"));
  }
  else if (tsDisplayStep == 1) {
    lcd.print(F("Cloud Gas: "));
    lcd.print(receivedTSData.gas);
    lcd.setCursor(0, 1);
    lcd.print(F("Dist: "));
    lcd.print(receivedTSData.dist);
    lcd.print(F("cm"));
  }
  else if (tsDisplayStep == 2) {
    lcd.print(F("Cloud P/R: "));
    lcd.print(receivedTSData.pitch, 0);
    lcd.setCursor(0, 1);
    lcd.print(receivedTSData.roll, 0);
    lcd.print(F(" deg"));
  }
  else if (tsDisplayStep == 3) {
    lcd.print(F("Cloud Yaw: "));
    lcd.print(receivedTSData.yaw, 0);
    lcd.setCursor(0, 1);
    lcd.print(F("State: "));
    lcd.print(receivedTSData.state);
  }
  
  tsDisplayStep++;
  if (tsDisplayStep > 3) tsDisplayStep = 0;
}
