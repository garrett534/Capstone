#include <Wire.h>
const uint8_t MPU_ADDR = 0x68; // I2C address of the MPU-6050
const int LED_PIN = 19; // Pin number of the LED

// Define accelerometer values here for plotter
int accel_x;
int accel_y;
int accel_z;

void setup() {
  Serial.begin(9600); // Initialize serial communication
  Wire.begin(); // Initialize I2C communication
  Wire.beginTransmission(MPU_ADDR); // Start communication with the MPU-6050
  Wire.write(0x6B); // Send power management register address
  Wire.write(0); // Set the register to 0 (awake mode)
  Wire.endTransmission(true); // End communication
  pinMode(LED_PIN, OUTPUT); // Set LED pin as output
}

void loop() {
  int16_t accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z;
  Wire.beginTransmission(MPU_ADDR); // Start communication with the MPU-6050
  Wire.write(0x3B); // Send first register of the accelerometer data
  Wire.endTransmission(false); // Restart communication without releasing the bus
  Wire.requestFrom(MPU_ADDR, (size_t) 14, true); // Request 14 bytes of data from the MPU-6050
  accel_x = Wire.read() << 8 | Wire.read();
  accel_y = Wire.read() << 8 | Wire.read();
  accel_z = Wire.read() << 8 | Wire.read();
  Wire.read(); // Skip temperature data
  Wire.read();
  gyro_x = Wire.read() << 8 | Wire.read();
  gyro_y = Wire.read() << 8 | Wire.read();
  gyro_z = Wire.read() << 8 | Wire.read();
  Serial.print("Accelx:");
  Serial.print(accel_x);
  Serial.print(",");
  Serial.print("Accely:");
  Serial.print(accel_y);
  Serial.print(",");
  Serial.print("Accelz:");
  Serial.print(accel_z);
  Serial.print("\n");

  //Serial.print(" | Gyroscope: ");
  //Serial.print(gyro_x);
  //Serial.print(", ");
  //Serial.print(gyro_y);
  //Serial.print(", ");
  //Serial.println(gyro_z);
  
  if (accel_z < 3) {
    digitalWrite(LED_PIN, HIGH); // Turn on LED if device is flipped 180º
  } else {
    digitalWrite(LED_PIN, LOW); // Turn off LED if device is not flipped 180º
  }
  
  delay(50); // Wait for 500ms before reading again
}
