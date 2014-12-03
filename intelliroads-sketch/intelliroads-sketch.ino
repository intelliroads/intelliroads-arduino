/*
  Intelliroads
 
 Demonstrates the use of reed switches wired to the Arduino board,
 in this to calculate the speed of the magnet an make a POST to a 
 RESTful API. The reed switches are separated by a known distance,
 which combined with the closing time of each switch, is used by
 Arduino to calculate the speed of the magnet. Finally, the Arduino
 makes a POST with the payload containing the speed value.
 
 The circuit:
 WiFi shield.
 Reed switches attached from 5V to pins 7 and 8, through 10k ohm pull-down resistors.
 
 created 2014
 by Alfredo El Ters, Diego Muracciole, Mathias Cabano, Matias Olivera y Santiago Caamano
 
 */

#include <SPI.h>
#include <WiFi.h>

const String highwayId = "IB";      // the id of the highway
const unsigned int spotId = 48200;      // the id (meters from the 0km) of the spot
const int laneId = 2;      // the lane id

char ssid[] = "Matias's iPhone";      //  network SSID (name)
char pass[] = "intelliroads";      // network password

int status = WL_IDLE_STATUS;

// Initialize the Wifi client library
WiFiClient client;

// server address:
IPAddress server(172,20,10,2);

const int switchAPin = 7;      // the pin that the reed switch A is attached to
const int switchBPin = 8;      // the pin that the reed switch B is attached to
const float distance = 0.10;      // distance between reed switches (0.10 m)

int switchAReading;
int switchBReading;
unsigned long switchATime;
unsigned long switchBTime;

void setup() {
  // initialize Serial:
  Serial.begin(9600);
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }
  
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  
  // initialize the reed switches pins as inputs:
  pinMode(switchAPin, INPUT);
  pinMode(switchBPin, INPUT);
}

void loop() {
  Serial.println("Waiting for switch A...");
  // get a switch A reading
  switchAReading = digitalRead(switchAPin);
  if (switchAReading == HIGH) {
    switchATime = millis();
    
    Serial.println("Waiting for switch B...");
    // wait for trigger of switch B to start calculation - otherwise timeout 
    do {
      switchBReading = digitalRead(switchBPin);
      if (millis() - switchATime > 5000) {
        return;
      }
    } while (switchBReading == LOW);
    
    switchBTime = millis();
    float timing = ((float) switchBTime - (float) switchATime)/1000;      // computes time in seconds
    float speed = distance/timing;      // computes speed in m/s
    postReading(speed);
  }
}

void postReading(float speed) {
  String data = "{ \"highwayId\": \"" + highwayId + "\", \"spotId\": " + spotId + ", \"laneId\": " + laneId + ", \"speed\": " + speed + " }";
  
  Serial.println(data);
  // close any connection before send a new request.
  // this will free the socket on the WiFi shield:
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 3000)) {
    Serial.println("connecting...");
    // send the HTTP POST request:
    client.println("POST /readings HTTP/1.1");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
    client.println();
  }
}
