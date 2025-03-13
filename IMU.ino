#include "Wire.h"  // Communicating with I2C devices

const int MPU_ADDR = 0x68;  // Default I2C address for MPU-6050

int16_t accelerometer_x, accelerometer_y, accelerometer_z;
int16_t gyro_x, gyro_y, gyro_z;
int16_t temperature;

const float ACCEL_SCALE = 16384.0;  // For ±2g range
const float GYRO_SCALE = 131.0;      // For ±250°/s range

char tmp_str[7];

// Convert int16_t to string with fixed length
char* convert_int16_to_str(int16_t i) {
    sprintf(tmp_str, "%6d", i);
    return tmp_str;
}

void setup() {
    Serial.begin(9600);
    Wire.begin();

    // Check MPU-6050 connection
    Wire.beginTransmission(MPU_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.println("Error: MPU-6050 not found!");
        while (1);  // Stop execution
    }
    Serial.println("MPU-6050 connected!");

    // Wake up MPU-6050
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission(true);
}

void readMPUData() {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);  // Start reading from ACCEL_XOUT_H
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 14, true);  // Request 14 registers

    if (Wire.available() == 14) {  // Ensure all data is received
        accelerometer_x = Wire.read() << 8 | Wire.read();
        accelerometer_y = Wire.read() << 8 | Wire.read();
        accelerometer_z = Wire.read() << 8 | Wire.read();
        temperature = Wire.read() << 8 | Wire.read();
        gyro_x = Wire.read() << 8 | Wire.read();
        gyro_y = Wire.read() << 8 | Wire.read();
        gyro_z = Wire.read() << 8 | Wire.read();
    } else {
        Serial.println("Error: Failed to read sensor data");
    }
}

void printSensorData() {
    // Print headers only once in the setup function
    static bool printedHeader = false;

    if (!printedHeader) {
        // Print the CSV headers
        Serial.println("Timestamp,aX (g),aY (g),aZ (g),Temp (C),gX (°/s),gY (°/s),gZ (°/s)");
        printedHeader = true;
    }

    // Print sensor data in CSV format
    long timestamp = millis();  // Current time in milliseconds
    Serial.print(timestamp);  // Timestamp
    Serial.print(",");  // Comma separator

    // Accelerometer data (scaled)
    Serial.print(accelerometer_x / ACCEL_SCALE);
    Serial.print(",");
    Serial.print(accelerometer_y / ACCEL_SCALE);
    Serial.print(",");
    Serial.print(accelerometer_z / ACCEL_SCALE);
    Serial.print(",");

    // Temperature data (scaled)
    Serial.print(temperature / 340.00 + 36.53);
    Serial.print(",");

    // Gyroscope data (scaled)
    Serial.print(gyro_x / GYRO_SCALE);
    Serial.print(",");
    Serial.print(gyro_y / GYRO_SCALE);
    Serial.print(",");
    Serial.print(gyro_z / GYRO_SCALE);

    // Print a newline to separate each data entry
    Serial.println();
}


void loop() {
    readMPUData();
    printSensorData();
    delay(100);
}
