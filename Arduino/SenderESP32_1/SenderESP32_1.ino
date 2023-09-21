#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>

// Reciever Board Code

uint8_t broadcastAddress[] = {0x53, 0x43, 0xB2, 0x2B, 0x4E, 0xB4};

typedef struct struct_message {
  int id;
  int x;
  int y;
}struct_message;

// Create a struct_message called myData
struct_message myData;

// Create peer interface
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// const uint8_t MPU_ADDR = 0x68; // I2C address of the MPU-6050
const int xPin = 2;
const int yPin = 3;
const int zPin = 4;

// Create the objects for server and client
WiFiServer server(80);
WiFiClient client;

const char* ssid   = "ROAR WebServer";// This is the SSID that ESP32 will broadcast
const char* password = "12345678";     // password should be atleast 8 characters to make it work

// Create the global variable
int16_t accel_x;
int16_t accel_y;

// Variable to store the HTTP request
String header;

void sendResponse() {
  // Send the HTTP response headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
}

void updateWebpage() {

  // Send the whole HTML
  client.println("<!DOCTYPE html><html>");
  client.println("<head>");
  client.println("<meta http-equiv=\"refresh\" content=\"1\">");
  client.println("<title>ESP32 Accelerometer Sensor</title>");
  client.println("</head>");

  // Web Page Heading
  client.println("<body><h1>ESP32 Accelerometer Sensor</h1>");

  //X
  client.println("<p>1. X: " + String(accel_x) + "</p>");
  client.print("<hr>");

  //Y
  client.println("<p>2. Y: " + String(accel_y) + "</p>");
  client.print("<hr>");

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
  
  // Create the ESP32 access point
  WiFi.softAP(ssid, password);

  Serial.println( "" );
  Serial.println( "WiFi AP is now running" );
  Serial.println( "IP address: " );
  Serial.println( WiFi.softAPIP() );

  // Start our ESP32 server
  server.begin();
}

void loop() {

  myData.id = 1;
  myData.x = analogRead(xPin);
  myData.y = analogRead(yPin);
  
  /*
  Serial.print("Accelx:");
  Serial.print(accel_x);
  Serial.print(",");
  Serial.print("Accely:");
  Serial.print(accel_y);
  Serial.print(",");

  */

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  
  if ( client = server.available() ) {  // Checks if a new client tries to connect to our server
    Serial.println("New Client.");
    String clientData = "";    
    while ( client.connected() ) {    // Wait until the client finish sending HTTP request
      if ( client.available() ) {     // If there is a data,
        char c = client.read();      //  read one character
        header += c;            //  then parse it
        Serial.write(c);
        if (c == 'n') {         // If the character is carriage return,
          //  it means end of http request from client
          if (clientData.length() == 0) { //  Now that the clientData is cleared,
            sendResponse();        //    perform the necessary action
            updateWebpage();
            break;
          } else {
            clientData = "";       //  First, clear the clientData
          }
        } else if (c != 'r') {      // Or if the character is NOT new line
          clientData += c;        //  store the character to the clientData variable
        }
      }
    }
    header = "";
    
    client.stop();            // Disconnect the client.
    Serial.println("Client disconnected.");
    Serial.println("");
    
  }
 

  delay(50);
}
