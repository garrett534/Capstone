#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>

// Reciever Board Code

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
  int id;
  int x;
}struct_message;

// Create a struct_message called myData
struct_message myData;

// Create the global variable
double R2R1_diff; //difference of rower 2 w/respect to rower 1
double R3R1_diff; //difference of rower 3 w/respect to rower 1
double R4R1_diff; //difference of rower 4 w/respect to rower 1
int sync_thrs = 200; //threshold of 3 g's (not final waiting on testing)
int R2R1_sync = 0; //sync bits for rower 2, 1 for faster, 2 in sync, 3 for slower
int R3R1_sync; // sync bits for rower 3
int R4R1_sync; // sync bits for rower 4

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;
struct_message board4;

// Create an array with all the structures
struct_message boardsStruct[4] = {board1, board2, board3, board4};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  //Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  //Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  //Update the structures with the new incoming data
  boardsStruct[myData.id-1].x = myData.x; //add data into board1 
  Serial.printf("x value: %d \n", boardsStruct[myData.id-1].x);
  //Serial.printf("R2R1 Sync: %d \n", R2R1_sync);
  //compare rower 2 to rower 1
  R2R1_diff = boardsStruct[0].x - boardsStruct[1].x; //rower 1 - rower 2
  if(R2R1_diff <= sync_thrs && R2R1_diff >= -sync_thrs){ //rowers in sync
      R2R1_sync = 2;
    } else if (R2R1_diff < -sync_thrs){ //rower2 faster
      R2R1_sync = 1;
    } else { //rower 2 slower 
      R2R1_sync = 3;
    }
  //compare rower 3 to rower 1
  R3R1_diff = boardsStruct[0].x - boardsStruct[2].x; //rower 1 - rower 3
  if(R3R1_diff <= sync_thrs && R3R1_diff >= -sync_thrs){ //rowers in sync
      R3R1_sync = 2;
    } else if (R3R1_diff < -sync_thrs){ //rower3 faster
      R3R1_sync = 1;
    } else { //rower 3 slower 
      R3R1_sync = 3;
    }
  //compare rower 4 to rower 1
  R4R1_diff = boardsStruct[0].x - boardsStruct[3].x; //rower 1 - rower 4
  if(R4R1_diff <= sync_thrs && R4R1_diff >= -sync_thrs){ //rowers in sync
      R4R1_sync = 2;
    } else if (R4R1_diff < -sync_thrs){ //rower4 faster
      R4R1_sync = 1;
    } else { //rower 4 slower 
      R4R1_sync = 3;
    }
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

  //rower 1
 // client.println("<p>1. X: " + String(accel_xR1[b]) + "</p>");
  //client.print("<hr>");

  //rower 2
  //client.println("<p>2. Y: " + String(accel_xR2[b]) + "</p>");
  //client.print("<hr>");
  //b++;

  //Z
  //client.println("<p>3. Z: " + String(accel_z) + "</p>");
  client.println("</body></html>");
  client.println();
}

void setup() {
  Serial.begin(115200);

  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  Serial.println();
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  
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

  //accel_x = analogRead(xPin);
  //accel_y = analogRead(yPin);
  //accel_z = analogRead(zPin);
  /*
  Serial.print("Accelx:");
  Serial.print(accel_x);
  Serial.print(",");
  Serial.print("Accely:");
  Serial.print(accel_y);
  Serial.print(",");
  Serial.print("Accelz:");
  Serial.print(accel_z);
  Serial.print("\n");
  */
  /*
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
 */

  delay(50);
}
