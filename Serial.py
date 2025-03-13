import serial
import time
import sys
import csv

# Check if the user provided an argument (axis)
if len(sys.argv) != 2 or sys.argv[1] not in ['x', 'y', 'z']:
    print("Usage: python script.py <axis>")
    print("<axis> should be one of 'x', 'y', or 'z'.")
    sys.exit(1)

# Get the axis to filter (x, y, or z) for the filename
axis = sys.argv[1]

# Set up the serial connection (adjust the COM port accordingly)
# On Windows, the port is typically something like "COM3"
# On macOS or Linux, it might be something like "/dev/ttyUSB0" or "/dev/ttyACM0"
arduino = serial.Serial('COM8', 9600)  # Replace with your Arduino port
time.sleep(2)  # Give the connection time to establish

# Open the CSV file based on the axis argument (it only affects the filename)
filename = f"{axis}_axis.csv"
with open(filename, 'w', newline='') as csvfile:
    csv_writer = csv.writer(csvfile)
    csv_writer.writerow(["Timestamp", "aX (g)", "aY (g)", "aZ (g)", "Temp (C)", "gX (°/s)", "gY (°/s)", "gZ (°/s)"])

    read_count = 0

    while read_count < 10000:
        # Read data from the serial port
        if arduino.in_waiting > 0:
            data = arduino.readline().decode('utf-8').strip()

            # Parse the data assuming it's in CSV format: Timestamp,aX,aY,aZ,Temp,gX,gY,gZ
            values = data.split(',')
            if len(values) >= 8:  # Ensure we have all the data (Timestamp, aX, aY, aZ, Temp, gX, gY, gZ)
                timestamp = values[0]
                sensor_x = values[1]
                sensor_y = values[2]
                sensor_z = values[3]
                temperature = values[4]
                gyro_x = values[5]
                gyro_y = values[6]
                gyro_z = values[7]

                # Write all sensor data to the CSV file
                csv_writer.writerow([timestamp, sensor_x, sensor_y, sensor_z, temperature, gyro_x, gyro_y, gyro_z])
                read_count += 1

                # Print the received data (optional)
                print(f"Received from Arduino: {data}")

    print(f"Data saved to {filename} ({read_count} reads)")
