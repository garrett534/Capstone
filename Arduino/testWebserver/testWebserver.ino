#include <ESPAsyncWebSrv.h>
#include <WiFi.h>
#include <Wire.h>
#include <esp_now.h>
#include <Arduino_JSON.h>

// Create the global variable
int counter = 0;
double R2R1_diff; //difference of rower 2 w/respect to rower 1
double R3R1_diff; //difference of rower 3 w/respect to rower 1
double R4R1_diff; //difference of rower 4 w/respect to rower 1
int sync_thrs = 150; //threshold of 3 g's (not final waiting on testing)
int R2R1_sync = 2; //sync bits for rower 2, 3 for slower, 1 for faster, 2 in sync
int R3R1_sync = 2; // sync bits for rower 3
int R4R1_sync = 2; // sync bits for rower 4
int rower1 = 1;
int rower2 = 2;
int rower3 = 3;
int rower4 = 4;

AsyncWebServer server(80);
AsyncEventSource events("/events");

// Reciever Board Code
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

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
  boardsStruct[myData.id-1].x = abs(myData.x);
  boardsStruct[myData.id-1].y = myData.y;
  Serial.printf("x value: %d \n", boardsStruct[myData.id-1].x);
  Serial.printf("y value: %d \n", boardsStruct[myData.id-1].y);

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
        <h4>Rower 1</h4>
        <div class="status-box" id="status1"></div>
        <p class="status-text" id="statusText1">Target Rower</p>
      </div>
      <div class="card">
        <h4>Rower 2</h4>
        <div class="status-box" id="status2"></div>
        <p class="status-text" id="statusText2">Status: <span id="statusSpan2"></span></p>
      </div>
      <div class="card">
        <h4>Rower 3</h4>
        <div class="status-box" id="status3"></div>
        <p class="status-text" id="statusText3">Status: <span id="statusSpan3"></span></p>
      </div>
      <div class="card">
        <h4>Rower 4</h4>
        <div class="status-box" id="status4"></div>
        <p class="status-text" id="statusText4">Status: <span id="statusSpan4"></span></p>
      </div>
    </div>
    <p id="count"></p>
  </div>
<script>
// Function to update the count on the webpage
    function updateCount(count) {
      var countElement = document.getElementById('count');
      countElement.innerText = count;
    }
  // Function to update the box color and status text with random colors
  function updateStatusColors(rower, value) {
    var statusBox = document.getElementById('status' + rower);
    var statusTextSpan = document.getElementById('statusSpan' + rower); // Updated

  // Assign colors based on the inSyncValue
    var color;
    var text; 
    
    if (value === 1) {
      color = 'yellow';
      text = 'faster';
    } else if (value === 2) {
      color = 'green';
      text = 'in-sync';
    } else if (value === 3) {
      color = 'red';
      text = 'slower';
    } 

    // Update the status box color
    statusBox.style.backgroundColor = color;
    statusTextSpan.textContent = text; 
  }

  // Function to update the status boxes periodically
  function updateStatusPeriodically() {
    // Create an event listener for the 'events' source
    var source = new EventSource('/events');

    // Event listener for the 'message' event
    source.addEventListener('message', function(e) {
      // Parse the data received from the server
      var data = e.data.split(',');
      var R2R1_sync = parseInt(data[0]);
      var R3R1_sync = parseInt(data[1]);
      var R4R1_sync = parseInt(data[2]);
      var count = data[3]; // Get the count from the data

      // Update the status boxes and the count based on the received data
      updateStatusColors(2, R2R1_sync);
      updateStatusColors(3, R3R1_sync);
      updateStatusColors(4, R4R1_sync);
      updateCount(count); // Update the count on the webpage
    }, false);
  }

  // Call the function to update the status initially
  updateStatusPeriodically();

  // Update the status boxes every 1 seconds (adjust the interval as needed)
  setInterval(updateStatusPeriodically, 1000);
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

// Function to update the status boxes periodically
void updateStatus() {
  // Increment the counter
  counter++;
  // Convert the counter to a string for display
  String countStr = "Timer: " + String(counter);

  // Update the event source with the count
  String data = String(R2R1_sync) + "," + String(R3R1_sync) + "," + String(R4R1_sync) + "," + countStr;
  events.send(data.c_str(), NULL, millis());
}

void loop() {
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 1000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    events.send("ping",NULL,millis());
    updateStatus();
    lastEventTime = millis();
  }
}
