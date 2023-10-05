#include <ESPAsyncWebSrv.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>
#include <Arduino_JSON.h>

AsyncWebServer server(80);
AsyncEventSource events("/events");

// Reciever Board Code
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

int inSync1 = 1;
int inSync2 = 1;
int inSync3 = 1;
int inSync4 = 1;

// Create the global variable
int16_t accel_x;
int16_t accel_y;

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
struct_message board4;

// Create an array with all the structures
struct_message boardsStruct[4] = {board1, board2, board3, board4};

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
  accel_x = myData.x;
  accel_y = myData.y;
  Serial.println();
}

// const uint8_t MPU_ADDR = 0x68; // I2C address of the MPU-6050
const int xPin = 2;
const int yPin = 3;
const int zPin = 4;

const char* ssid   = "ROAR WebServer1";// This is the SSID that ESP32 will broadcast
const char* password = "12345678";     // password should be atleast 8 characters to make it work

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ROAR Sync Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    p {font-size: 1.2rem;}
    body {margin: 0;}
    .topnav {overflow: hidden; background-color: #e33636; color: white; font-size: 1.7rem;}
    .content {padding: 20px;}
    .card {background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); margin: 10px; padding: 10px; text-align: center; width: 200px;}
    .cards {max-width: 700px; margin: 0 auto; display: flex; flex-direction: row; justify-content: center;align-items: center;}
    .reading {font-size: 2.8rem;}
    .packet {color: #bebebe;}
    .status-box {width: 100px; height: 100px; border: 2px solid #000; margin: 0 auto;}
    .status-text {font-size: 1.5rem; margin-top: 10px;}
  </style>
</head>
<body>
  <div class="topnav">
    <h3>ROAR Sync Dashboard</h3>
  </div>
  <div class="content">
    <div class="cards">
      <div class="card">
        <h4><i class="fas fa-thermometer-half"></i> Rower 1</h4>
        <div class="status-box" id="status1"></div>
        <p class="status-text" id="statusText1">Status: <span id="statusSpan1"></span></p>
      </div>
      <div class="card">
        <h4><i class="fas fa-tint"></i> Rower 2</h4>
        <div class="status-box" id="status2"></div>
        <p class="status-text" id="statusText2">Status: <span id="statusSpan2"></span></p>
      </div>
      <div class="card">
        <h4><i class="fas fa-thermometer-half"></i> Rower 3</h4>
        <div class="status-box" id="status3"></div>
        <p class="status-text" id="statusText3">Status: <span id="statusSpan3"></span></p>
      </div>
      <div class="card">
        <h4><i class="fas fa-tint"></i> Rower 4</h4>
        <div class="status-box" id="status4"></div>
        <p class="status-text" id="statusText4">Status: <span id="statusSpan4"></span></p>
      </div>
    </div>
  </div>
  <script>
  // Function to update the box color and status text with random colors
  function updateStatusRandomColors(rower) {
    var statusBox = document.getElementById('status' + rower);
    var statusTextSpan = document.getElementById('statusSpan' + rower); // Updated

    var colors = ['green', 'red', 'yellow'];
    var randomColor = colors[Math.floor(Math.random() * colors.length)];
    
    statusBox.style.backgroundColor = randomColor;
    
    // Set the appropriate status text based on the color
    if (randomColor === 'green') {
      statusTextSpan.textContent = 'in-sync'; // Updated
    } else if (randomColor === 'red') {
      statusTextSpan.textContent = 'out-of-sync'; // Updated
    } else {
      statusTextSpan.textContent = 'warning'; // Updated
    }
  }

  // Function to update the status boxes periodically
  function updateStatusPeriodically() {
    updateStatusRandomColors(1);
    updateStatusRandomColors(2);
    updateStatusRandomColors(3);
    updateStatusRandomColors(4);
  }

  // Call the function to update the status initially
  updateStatusPeriodically();

  // Update the status boxes every 3 seconds (adjust the interval as needed)
  setInterval(updateStatusPeriodically, 1500);
</script>
</body>
</html>)rawliteral";

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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  events.onConnect([](AsyncEventSourceClient *client){
      if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  // Start our ESP32 server
  server.begin();

}

void loop() {
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 5000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping",NULL,millis());
    lastEventTime = millis();
  }
}
