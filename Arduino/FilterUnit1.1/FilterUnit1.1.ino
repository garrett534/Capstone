#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>

// Mapping code
// initialize minimum and maximum Raw Ranges for each axis
int RawMin = 0;
int RawMax = 3300;

// Define global variables
const int xPin = 2;
int unfilteredWrongReading;

// Take multiple samples to reduce noise
const int sampleSize = 10;

// Define a previous read
int previousRead = analogRead(xPin); 

// Take samples and return the average
int ReadAxis(int axisPin)
{
  // Declare variables
  int reading = 0;
  int currentRead = 0;
  unfilteredWrongReading = 0;
  
  for (int i = 0; i < sampleSize; i++)
  {
  currentRead = analogRead(axisPin);

  // Insert fake bad sample > 3300 or < 0
  if (i == 5){
    currentRead += 10000;
  }

  // Updated unfilteredWrongReading
  unfilteredWrongReading += currentRead;
  
  // Check for bad sample
  if (currentRead > 3300 || currentRead < 0){ //Note: this will have to change with the calibration code!!
    currentRead = previousRead; 
  }
  reading += currentRead;
  previousRead = currentRead; 
  }

  // Average the unfilteredWrongReading
  unfilteredWrongReading = unfilteredWrongReading/sampleSize;
  
  return reading/sampleSize;
}

// Reciever Board Code
uint8_t broadcastAddress[] = {0x53, 0x43, 0xB2, 0x2B, 0x4E, 0xB4};

typedef struct struct_message {
  int id;
  float x;
  float y;
}struct_message;

// Create a struct_message called myData
struct_message myData;

// Create the global variable
double accel_x;

void setup() {
  Serial.begin(115200);
}

void loop() {

  myData.id = 2;
  //myData.x = analogRead(xPin);
  //Read raw values
  int xFiltered = ReadAxis(xPin);
  int xUnfiltered = unfilteredWrongReading;
  
  Serial.print("xFiltered:");
  Serial.print(xFiltered);
  Serial.print(",");
  Serial.print("unFiltered:");
  Serial.print(xUnfiltered);
  Serial.println(",");
  
}
