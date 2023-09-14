
// Load Wi-Fi library
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>

// Replace with your network credentials
const char* ssid = "ESP32WebServer5";
const char* password = "12345678";

int lcdColumns = 16;
int lcdRows = 2;

String inputBox;

// Set web server port number to 80
WiFiServer server(80);
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output33State = "off";
String output27State = "off";

// Assign output variables to GPIO pins
const int output33 = 5;
const int output27 = 12;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output33, OUTPUT);
  pinMode(output27, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output33, LOW);
  digitalWrite(output27, LOW);

  //Initalize and turn on backlight
  lcd.init();
  lcd.backlight();

  // Connect to Wi-Fi network with SSID and password
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                         	// If a new client connects,
	currentTime = millis();
	previousTime = currentTime;
	Serial.println("New Client.");      	// print a message out in the serial port
	printToScreen("Connected");
	String currentLine = "";            	// make a String to hold incoming data from the client
	while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
  	currentTime = millis();
  	if (client.available()) {         	// if there's bytes to read from the client,
    	char c = client.read();         	// read a byte, then
    	Serial.write(c);                	// print it out the serial monitor
    	header += c;
    	if (c == '\n') {                	// if the byte is a newline character
      	// if the current line is blank, you got two newline characters in a row.
      	// that's the end of the client HTTP request, so send a response:
      	if (currentLine.length() == 0) {
        	// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
        	// and a content-type so the client knows what's coming, then a blank line:
        	client.println("HTTP/1.1 200 OK");
        	client.println("Content-type:text/html");
        	client.println("Connection: close");
        	client.println();
       	 
        	// turns the GPIOs on and off
        	if (header.indexOf("GET /33/on") >= 0) {
          	Serial.println("GPIO 33 on");
          	printToScreen("GPIO 33 on");
          	output33State = "on";
          	digitalWrite(output33, HIGH);
        	} else if (header.indexOf("GET /33/off") >= 0) {
          	Serial.println("GPIO 33 off");
          	printToScreen("GPIO 33 off");
          	output33State = "off";
          	digitalWrite(output33, LOW);
        	} else if (header.indexOf("GET /27/on") >= 0) {
          	Serial.println("GPIO 27 on");
          	printToScreen("GPIO 27 on");
          	output27State = "on";
          	digitalWrite(output27, HIGH);
        	} else if (header.indexOf("GET /27/off") >= 0) {
          	Serial.println("GPIO 27 off");
          	printToScreen("GPIO 27 off");
          	output27State = "off";
          	digitalWrite(output27, LOW);
        	}
       	 
        	// Display the HTML web page
        	client.println("<!DOCTYPE html><html>");
        	client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
        	client.println("<link rel=\"icon\" href=\"data:,\">");
        	// CSS to style the on/off buttons
        	// Feel free to change the background-color and font-size attributes to fit your preferences
        	client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
        	client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
        	client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
        	client.println(".button2 {background-color: #555555;}</style></head>");
       	 
       	 
        	// Web Page Heading
        	client.println("<body><h1>ESP32 Web Server</h1>");
       	 
        	// Display current state, and ON/OFF buttons for GPIO 26  
        	client.println("<p>GPIO 33 - State " + output33State + "</p>");
        	// If the output33State is off, it displays the ON button  	 
        	if (output33State=="off") {
          	client.println("<p><a href=\"/33/on\"><button class=\"button\">ON</button></a></p>");
        	} else {
          	client.println("<p><a href=\"/33/off\"><button class=\"button button2\">OFF</button></a></p>");
        	}
          	 
        	// Display current state, and ON/OFF buttons for GPIO 27  
        	client.println("<p>GPIO 27 - State " + output27State + "</p>");
        	// If the output27State is off, it displays the ON button  	 
        	if (output27State=="off") {
          	client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
        	} else {
          	client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
        	}

        	client.println("<form action=\"/get\"> input1: <input type=\"text\" name=\"input1\"/><input type=\"submit\" value=\"Submit\"></form><br>");
       	 
       	 
        	client.println("</body></html>");
       	 
        	// The HTTP response ends with another blank line
        	client.println();
        	// Break out of the while loop
        	break;
      	} else { // if you got a newline, then clear currentLine
        	currentLine = "";
      	}
    	} else if (c != '\r') {  // if you got anything else but a carriage return character,
      	currentLine += c;  	// add it to the end of the currentLine
    	}
  	}
	}
	// Clear the header variable
	header = "";
	// Close the connection
	client.stop();
	Serial.println("Client disconnected.");
	Serial.println("");
  }
}

void printToScreen(String message){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message.substring(0,15));
  lcd.setCursor(0,1);
  lcd.print(message.substring(16,31));

}