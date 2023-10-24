#include <WiFi.h>

// const uint8_t MPU_ADDR = 0x68; // I2C address of the MPU-6050

// ADC pins to 
const int ADCPin0 = 0;
const int ADCPin1 = 1;
const int ADCPin2 = 2;
const int ADCPin3 = 3;
const int ADCPin4 = 4;
const int ADCPin5 = 5;

// Create the objects for server and client
WiFiServer server(80);
WiFiClient client;

const char* ssid   = "ROAR WebServer";// This is the SSID that ESP32 will broadcast
const char* password = "12345678";     // password should be atleast 8 characters to make it work

// Create the global variable
int16_t accel_x_0;
int16_t accel_x_1;
int16_t accel_x_2;
int16_t accel_x_3;
int16_t accel_x_4;
int16_t accel_x_5;


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
/*
  //X
  client.println("<p>1. X: " + String(accel_x) + "</p>");
  client.print("<hr>");

  //Y
  client.println("<p>2. Y: " + String(accel_y) + "</p>");
  client.print("<hr>");

  //Z
  client.println("<p>3. Z: " + String(accel_z) + "</p>");
  client.println("</body></html>");
  */
  client.println();
}

void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);

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
  accel_x_0 = analogRead(ADCPin0);
  accel_x_1 = analogRead(ADCPin1);
  accel_x_2 = analogRead(ADCPin2);
  accel_x_3 = analogRead(ADCPin3);
  accel_x_4 = analogRead(ADCPin4);
  accel_x_5 = analogRead(ADCPin5);
  
  
  Serial.print("ADC Pin 0:");
  Serial.print(accel_x_0);
  Serial.print(",");
  Serial.print("ADC Pin 1:");
  Serial.print(accel_x_1);
  Serial.print(",");
  Serial.print("ADC Pin 2:");
  Serial.print(accel_x_2);

  Serial.print("ADC Pin 3:");
  Serial.print(accel_x_3);
  Serial.print(",");
  Serial.print("ADC Pin 4:");
  Serial.print(accel_x_4);
  Serial.print(",");
  Serial.print("ADC Pin 5:");
  Serial.print(accel_x_5);
  
  Serial.print("\n");

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
