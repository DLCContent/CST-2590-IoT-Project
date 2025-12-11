#!/usr/bin/env python3
"""
===================================================
ThingSpeak Bidirectional Script - Disaster Recon UAV
===================================================

Student ID: M00851824
Course: CST2590 - IoT Cloud (Lecture 9)

This script:
1. Reads sensor data from Arduino via Serial
2. Uploads data to ThingSpeak (WRITE)
3. Waits for data to be available
4. Reads data back from ThingSpeak (READ)
5. Sends retrieved data back to Arduino

Date: 2025-12-10
===================================================
"""

import serial
import requests
import time
import sys
from datetime import datetime

# ===================================================
# CONFIGURATION
# ===================================================

# Import sensitive credentials from separate file
from channel_info import (
    THINGSPEAK_WRITE_API_KEY,
    THINGSPEAK_READ_API_KEY,
    THINGSPEAK_CHANNEL_ID,
    STUDENT_ID
)

# Use imported credentials
ID = STUDENT_ID
CHANNEL_ID = THINGSPEAK_CHANNEL_ID
WRITE_API_KEY = THINGSPEAK_WRITE_API_KEY
READ_API_KEY = THINGSPEAK_READ_API_KEY

# Local configuration (not sensitive)
COM_PORT = 'COM7'
BAUD_RATE = 9600
UPDATE_INTERVAL = 20  # seconds (must be >= 15 for ThingSpeak free tier)
READ_DELAY = 5  # seconds to wait after write before reading

# ===================================================
# HELPER FUNCTIONS
# ===================================================

def print_header():
    """Print startup header"""
    print("=" * 60)
    print(" DISASTER RECON UAV - ThingSpeak Bidirectional Test")
    print(" Student ID:", ID)
    print("=" * 60)
    print(f"ThingSpeak Channel: {CHANNEL_ID}")
    print(f"Serial Port: {COM_PORT} @ {BAUD_RATE} baud")
    print(f"Update Interval: {UPDATE_INTERVAL} seconds")
    print("=" * 60)
    print()

def connect_serial():
    """Establish serial connection to Arduino"""
    try:
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=2)
        time.sleep(2)  # Wait for Arduino to initialize
        print(f"✓ Connected to Arduino on {COM_PORT}")
        return ser
    except serial.SerialException as e:
        print(f"✗ Error connecting to {COM_PORT}: {e}")
        print("  Check COM port and ensure Arduino is connected")
        return None

def parse_csv_data(line):
    """Parse CSV data from Arduino"""
    try:
        # CSV format: temp,humid,gas,dist,state,pitch,roll,yaw
        values = line.split(',')
        
        if len(values) != 8:
            print(f"✗ Invalid CSV (expected 8 values, got {len(values)}): {line}")
            return None
        
        data = {
            'temperature': float(values[0]),
            'humidity': float(values[1]),
            'gas_level': int(values[2]),
            'distance': int(values[3]),
            'drone_state': int(values[4]),
            'pitch': float(values[5]),
            'roll': float(values[6]),
            'yaw': float(values[7])
        }
        return data
    except (ValueError, IndexError) as e:
        print(f"✗ Error parsing CSV: {e} - {line}")
        return None

def upload_to_thingspeak(data):
    """Upload sensor data to ThingSpeak"""
    try:
        url = f"https://api.thingspeak.com/update?api_key={WRITE_API_KEY}"
        
        # Build field parameters
        params = {
            "field1": data['temperature'],   # Temperature
            "field2": data['humidity'],      # Humidity
            "field3": data['gas_level'],     # Gas Level
            "field4": data['distance'],      # Distance
            "field5": data['drone_state'],   # Drone State
            "field6": data['pitch'],         # Pitch
            "field7": data['roll'],          # Roll
            "field8": data['yaw']            # Yaw
        }
        
        # Send HTTP GET request
        response = requests.get(url, params=params, timeout=10)
        
        if response.status_code == 200 and response.text != "0":
            return True, response.text
        else:
            return False, f"Upload failed: {response.text}"
            
    except requests.exceptions.RequestException as e:
        return False, f"Network error: {e}"

def read_from_thingspeak():
    """Read latest data from ThingSpeak"""
    try:
        url = f"https://api.thingspeak.com/channels/{CHANNEL_ID}/feeds.json?api_key={READ_API_KEY}&results=1"
        
        response = requests.get(url, timeout=10)
        data = response.json()
        
        if not data.get("feeds") or not data["feeds"]:
            print("   ⚠️  No data found in ThingSpeak")
            return None
        
        # Extract the latest feed entry
        latest = data["feeds"][0]
        
        # Build data list in order (field1 through field8)
        retrieved_data = []
        for i in range(1, 9):
            field_key = f"field{i}"
            value = latest.get(field_key)
            if value is not None:
                retrieved_data.append(str(value))
            else:
                retrieved_data.append("0")  # Default if missing
        
        return retrieved_data
        
    except requests.exceptions.RequestException as e:
        print(f"   ✗ HTTP Request Error: {e}")
        return None
    except Exception as e:
        print(f"   ✗ Error processing ThingSpeak data: {e}")
        return None

def send_to_arduino(ser, data_list):
    """Send data to Arduino via serial"""
    try:
        # Format: value1|value2|value3|...|value8\r\n
        data_string = "|".join(data_list) + "\r\n"
        ser.write(data_string.encode('utf-8'))
        return True
    except Exception as e:
        print(f"   ✗ Error sending to Arduino: {e}")
        return False

# ===================================================
# MAIN PROGRAM
# ===================================================

def main():
    print_header()
    
    # Connect to Arduino
    ser = connect_serial()
    if not ser:
        print("\n✗ Failed to connect. Exiting...")
        return
    
    print("✓ Waiting for data from Arduino...")
    print("  (Make sure UAV is turned ON via IR remote)")
    print()
    
    last_upload_time = 0
    
    try:
        while True:
            # Read line from Serial
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8').strip()
                
                # Check if it's CSV data (contains numbers and commas)
                if ',' in line and not line.startswith('['):
                    current_time = time.time()
                    
                    # Check if enough time has passed for upload
                    if current_time - last_upload_time >= UPDATE_INTERVAL:
                        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        print(f"\n[{timestamp}] === NEW CYCLE ===")
                        
                        data = parse_csv_data(line)
                        
                        if data:
                            # STEP 1: Upload to ThingSpeak
                            print(f"  📤 UPLOADING to ThingSpeak...")
                            print(f"     Temp={data['temperature']}°C, Humid={data['humidity']}%, Gas={data['gas_level']}, Dist={data['distance']}cm")
                            
                            success, message = upload_to_thingspeak(data)
                            
                            if success:
                                print(f"  ✓ Upload Success (Entry: {message})")
                                last_upload_time = current_time
                                
                                # STEP 2: Wait for data to be available
                                print(f"  ⏳ Waiting {READ_DELAY} seconds for data to be available...")
                                time.sleep(READ_DELAY)
                                
                                # STEP 3: Read back from ThingSpeak
                                print(f"  📥 READING from ThingSpeak...")
                                retrieved_data = read_from_thingspeak()
                                
                                if retrieved_data:
                                    print(f"  ✓ Read Success: {' | '.join(retrieved_data)}")
                                    
                                    # STEP 4: Send to Arduino
                                    print(f"  📡 Sending to Arduino...")
                                    if send_to_arduino(ser, retrieved_data):
                                        print(f"  ✓ Sent to Arduino successfully!")
                                    else:
                                        print(f"  ✗ Failed to send to Arduino")
                                else:
                                    print(f"  ✗ Failed to read from ThingSpeak")
                            else:
                                print(f"  ✗ Upload Failed: {message}")
                        
                    else:
                        wait_time = UPDATE_INTERVAL - (current_time - last_upload_time)
                        print(f"  ⏱  Next cycle in {wait_time:.0f} seconds", end='\r')
                
                else:
                    # Print non-CSV lines (debug messages from Arduino)
                    if line.strip():
                        print(f"[Arduino] {line}")
            
            time.sleep(0.1)  # Small delay to prevent CPU overuse
            
    except KeyboardInterrupt:
        print("\n\n✓ Program interrupted by user")
        print("  Closing serial connection...")
        
    finally:
        if ser and ser.is_open:
            ser.close()
            print("✓ Serial connection closed")
        print("\nGoodbye!")

# ===================================================
# ENTRY POINT
# ===================================================

if __name__ == "__main__":
    main()
