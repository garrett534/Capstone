#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>


// Set Pins
const int xPin = 2;

// initialize minimum and maximum Raw Ranges for each axis
int RawMin = -1650;
int RawMax = 1650;

// Take multiple samples to reduce noise
const int sampleSize = 20;

// Define a previous read
int previousRead = 0; 

// Take samples and return the average
int ReadAxis(int axisPin)
{
  // Declare variables
  int reading = 0;
  int currentRead = 0;
  
  for (int i = 0; i < sampleSize; i++)
  {
  currentRead = analogRead(axisPin);
  
  // Check for bad sample
  if (currentRead > RawMax || currentRead < RawMin){
    currentRead = previousRead; 
  }
  reading += currentRead;
  previousRead = currentRead; 
  }
  
  return reading/sampleSize;
}

// Reciever Board Code

uint8_t broadcastAddress[] = {0x53, 0x43, 0xB2, 0x2B, 0x4E, 0xB4};

typedef struct struct_message {
  int id;
  int x;
  int y;
}struct_message;
//global variables
int offset=0;
int numSamples = 2000;
int sum_accel=0;
int i=0;
int calb;
// Create a struct_message called myData
struct_message myData;

// Create peer interface
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);

  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  
  for (i;i<=numSamples;i++){ //calibrate accelerometer
    calb = analogRead(xPin);
    sum_accel += calb;
    offset = sum_accel/numSamples;
  }

  // Update the previous read
  previousRead = analogRead(xPin);

  myData.id = 1;
  myData.x =  ReadAxis(xPin)-offset;
   
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  // Print myData.x for debugging 
  /*
  Serial.print("x Data:");
  Serial.print(myData.x);
  Serial.print(",");
  */ 
  
  Serial.println();
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  
  delay(50);
}
