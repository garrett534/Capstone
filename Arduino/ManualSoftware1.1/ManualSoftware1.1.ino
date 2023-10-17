
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>

// Reciever Board Code
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Create the global variables for the x-axis acceleration for each board
int16_t accel_x_1;
int16_t accel_x_2;
int16_t accel_x_3;
String accelString1;
String accelString2;
String accelString3;

typedef struct struct_message {
  int id;
  int x;
  int y;
}struct_message;

// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;
struct_message board3;

// Create an array with all the structures
struct_message boardsStruct[3] = {board1, board2, board3};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].x = myData.x;
  boardsStruct[myData.id-1].y = myData.y;
  Serial.printf("x value: %d \n", boardsStruct[myData.id-1].x);
  Serial.printf("y value: %d \n", boardsStruct[myData.id-1].y);

  // Check which board is sending data and save to the right global
  if(myData.id == 1){
    accel_x_1 = myData.x;
    accelString1 = "Accelx:" + String(accel_x_1) + "\n";
    appendFile(SD, "/ManualSoftware1.1_Board1/run.txt", accelString1.c_str());
  } else if(myData.id == 2){
    accel_x_2 = myData.x;
    accelString2 = "Accelx:" + String(accel_x_2) + "\n";
    appendFile(SD, "/ManualSoftware1.1_Board2/run.txt", accelString2.c_str());
  } else if(myData.id == 3){
    accel_x_3 = myData.x;
    accelString3 = "Accelx:" + String(accel_x_3) + "\n";
    appendFile(SD, "/ManualSoftware1.1_Board3/run.txt", accelString3.c_str());
  }
 
  Serial.println();
}

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

  //X
  //client.println("<p>1. X: " + String(accel_x) + "</p>");
  //client.print("<hr>");

  //Y
  //client.println("<p>2. Y: " + String(accel_y) + "</p>");
  //client.print("<hr>");

}

#define SCK  4
#define MISO  5
#define MOSI  6
#define CS  7
SPIClass spi = SPIClass('VSPI');

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}


void setup() {
  Serial.begin(115200);
  
    // Set SPI Pins
    spi.begin(SCK, MISO, MOSI, CS);
    
    if(!SD.begin(CS)){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    //listDir(SD, "/", 0);
    createDir(SD, "/ManualSoftware1.1_Board1");
    createDir(SD, "/ManualSoftware1.1_Board2");
    createDir(SD, "/ManualSoftware1.1_Board3");
    //listDir(SD, "/", 0);
    //removeDir(SD, "/mydir");
    //listDir(SD, "/", 2);
    writeFile(SD, "/ManualSoftware1.1_Board1/run.txt", "Running Test\n");
    writeFile(SD, "/ManualSoftware1.1_Board2/run.txt", "Running Test\n");
    writeFile(SD, "/ManualSoftware1.1_Board3/run.txt", "Running Test\n");
    //appendFile(SD, "/hello.txt", "World!\n");
    //readFile(SD, "/hello.txt");
    //deleteFile(SD, "/foo.txt");
    //renameFile(SD, "/hello.txt", "/foo.txt");
    //readFile(SD, "/foo.txt");
    //testFileIO(SD, "/test.txt");

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

    // Print remaining space
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    
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
