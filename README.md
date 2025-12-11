# Disaster Recon UAV - ThingSpeak Integration

Complete Arduino + Python system for disaster reconnaissance UAV with cloud telemetry.

## 📁 Project Files

### Arduino Files
- **`UAV_Telemetry.ino`** - Main Arduino sketch
- **`config.h`** - Pin definitions and constants
- **`sensors.h`** - Sensor reading functions
- **`actuators.h`** - LCD, LED, buzzer control
- **`motor_control.h`** - Motor and state machine
- **`IR_Code_Scanner.ino`** - Helper to find IR remote codes

### Python Files
- **`thingspeak_uploader.py`** - Main Python script
- **`config.py`** - ThingSpeak credentials and settings

---

## 🚀 Quick Start Guide

### Step 1: Install Arduino Libraries

Open Arduino IDE → Tools → Manage Libraries, search and install:

1. **DHT sensor library** by Adafruit
2. **LiquidCrystal I2C** by Frank de Brabander
3. **IRremote** by ArminJo
4. **MPU6050_tockn** by tockn

### Step 2: Find Your IR Remote Codes

1. Upload **`IR_Code_Scanner.ino`** to Arduino
2. Open Serial Monitor (9600 baud)
3. Press ON and OFF buttons on your remote
4. Note the hex codes displayed
5. Update `config.h` with your codes:
   ```cpp
   #define IR_BUTTON_ON    0xYOURCODE  // Replace
   #define IR_BUTTON_OFF   0xYOURCODE  // Replace
   ```

### Step 3: Configure ThingSpeak

1. Create free account at [thingspeak.com](https://thingspeak.com)
2. Create new channel: "Disaster Recon UAV Telemetry"
3. Add 8 fields:
   - Field 1: Temperature
   - Field 2: Humidity
   - Field 3: Gas Level
   - Field 4: Distance
   - Field 5: Drone State
   - Field 6: Pitch
   - Field 7: Roll
   - Field 8: Yaw
4. Note your **Channel ID** and **Write API Key**
5. Update `config.py`:
   ```python
   CHANNEL_ID = "YOUR_CHANNEL_ID"
   WRITE_API_KEY = "YOUR_WRITE_API_KEY"
   COM_PORT = "COM4"  # Your Arduino port
   ```

### Step 4: Upload Arduino Code

1. Open **`UAV_Telemetry.ino`** in Arduino IDE
2. Connect Arduino via USB
3. Select correct board: Tools → Board → Arduino Uno
4. Select correct port: Tools → Port → COM# (your port)
5. Click Upload ✓
6. Wait for "Done uploading"

### Step 5: Install Python Dependencies

Open Command Prompt or Terminal:

```bash
pip install pyserial requests
```

### Step 6: Run the System

1. Keep Arduino connected via USB
2. Run Python script:
   ```bash
   python thingspeak_uploader.py
   ```
3. Press **ON button** on IR remote to start UAV
4. Watch data upload to ThingSpeak every 20 seconds!

---

## 📊 ThingSpeak Fields

| Field | Sensor | Unit | Range |
|-------|--------|------|-------|
| 1 | Temperature | °C | 0-50 |
| 2 | Humidity | % | 0-100 |
| 3 | Gas Level | 0-1023 | Hazard ≥ 460 |
| 4 | Distance | cm | 2-400 |
| 5 | Drone State | 0-4 | OFF/IDLE/ACTIVE/ALERT/ERROR |
| 6 | Pitch | degrees | -90 to +90 |
| 7 | Roll | degrees | -180 to +180 |
| 8 | Yaw | degrees | 0 to 360 |

---

## 🎮 IR Remote Control

- **ON Button**: Starts motor, begins data collection
- **OFF Button**: Stops motor, stops data collection

---

## 🔊 Buzzer Alerts

- **1 beep**: Ready to fly / State change
- **3 beeps**: Hazard detected (gas ≥ 45%)

---

## 💡 RGB LED Status

- **Purple**: Ready/Idle
- **Green**: Normal operation
- **Blue**: Uploading data
- **Red**: Hazard alert
- **Yellow**: Sensor error
- **Off**: UAV powered off

---

## 🖥️ LCD Display

Rotates between 4 screens every 2 seconds:
1. Temperature, Humidity, Gas
2. Gas percentage, Distance
3. Pitch, Roll
4. Yaw

---

## 🔧 Troubleshooting

### Arduino won't compile
- Install all required libraries (Step 1)
- Make sure all `.h` files are in the same folder as `.ino`

### Python can't connect
- Check COM port in `config.py`
- Make sure Arduino is connected
- Try different COM port (COM3, COM5, etc.)

### ThingSpeak upload fails
- Check API key in `config.py`
- Verify internet connection
- Check ThingSpeak rate limit (15 sec minimum)

### IR remote not working
- Run `IR_Code_Scanner.ino` to find correct codes
- Update codes in `config.h`
- Check IR receiver wiring to pin D5

### Sensors not working
- Check wiring against pin assignments
- Test each sensor individually first
- Check Serial Monitor for error messages

---

## 📌 Pin Connections

| Component | Arduino Pin |
|-----------|-------------|
| IR Reciever | D2 |
| Motor Enable| D3 |
| Motor Input 1 | D4 
| Motor Input 2 | D5 |
| RGB LED Red | D6 |
| RGB LED Green | D7 |
| RGB LED Blue | D8 |
| Piezo Buzzer | D9 |
| Ultrasonic Trig | D10 |
| Ultrasonic Echo | D11 |
| DHT11 Data | D12 |
| MQ-2 Gas | A0 |
| LCD SDA | A4 |
| LCD SCL | A5 |
| MPU6050 SDA | D18 (SDA) |
| MPU6050 SCL | D19 (SCL) |
| MPU6050 AD0 | GND |

---

## 📝 Notes

- **Update Interval**: Default 20 seconds (configurable in `config.h`)
- **Memory Usage**: ~1200-1400 bytes SRAM, ~14 KB Flash
- **Gas Threshold**: 45% = ~460 on analog scale (configurable)
- **I2C Addresses**: LCD=0x27, MPU6050=0x68

---

## 📧 Support

If you encounter issues, check:
1. Serial Monitor output (Arduino IDE)
2. Python console output
3. ThingSpeak channel graphs
4. Wiring connections

Good luck with your UAV project! 🚁

