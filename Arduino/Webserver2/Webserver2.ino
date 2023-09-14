// Load Wi-Fi library

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Wire.h>

 

const uint8_t MPU_ADDR = 0x68; // I2C address of the MPU-6050

const int LED_PIN = 19; // Pin number of the LED

 

// Define accelerometer values here for plotter

int accel_x;

int accel_y;

int accel_z;

 

// Replace with your network credentials

const char* ssid     = "ROAR Webserver";

const char* password = "123456789";

 

// Set web server port number to 80

WiFiServer server(80);

 

// Variable to store the HTTP request

String header;

 

String readAccel() {

    int16_t accel_x, accel_y, accel_z;

    Wire.beginTransmission(MPU_ADDR); // Start communication with the MPU-6050

    Wire.write(0x3B); // Send first register of the accelerometer data

    Wire.endTransmission(false); // Restart communication without releasing the bus

    Wire.requestFrom(MPU_ADDR, (size_t) 14, true); // Request 14 bytes of data from the MPU-6050

    accel_x = Wire.read() << 8 | Wire.read();

    accel_y = Wire.read() << 8 | Wire.read();

    accel_z = Wire.read() << 8 | Wire.read();

    Wire.read(); // Skip temperature data

    Wire.read(); // Skip gyro data

    Serial.print("Accelx:");

    Serial.print(accel_x);

    Serial.print("\n");

    return String(accel_x);

}

 

void setup() {

  Serial.begin(9600); // Initialize serial communication

  Wire.begin(); // Initialize I2C communication

  Wire.beginTransmission(MPU_ADDR); // Start communication with the MPU-6050

  Wire.write(0x6B); // Send power management register address

  Wire.write(0); // Set the register to 0 (awake mode)

  Wire.endTransmission(true); // End communication

  pinMode(LED_PIN, OUTPUT); // Set LED pin as output

 

  // Connect to Wi-Fi network with SSID and password

  Serial.print("Setting AP (Access Point)…");

  // Remove the password parameter, if you want the AP (Access Point) to be open

  WiFi.softAP(ssid, password);

 

  IPAddress IP = WiFi.softAPIP();

  Serial.print("AP IP address: ");

  Serial.println(IP);

 

  server.begin();

 
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,

    Serial.println("New Client.");          // print a message out in the serial port

    String currentLine = "";                // make a String to hold incoming data from the client

 

    // Close the connection

    client.stop();

    Serial.println("Client disconnected.");

    Serial.println("");

  }

 



}

 

void loop(){

 

}
