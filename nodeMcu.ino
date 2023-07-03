#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include<SoftwareSerial.h>

SoftwareSerial s(D9,D10);

const char* ssid = "South Pole";
const char* password = "214110Gbdnm";

String serverName = "http://192.168.1.154:5000/api/";

ESP8266WebServer server(80);

double ph_value = 13.90;
double nh3_value = 1.234;

void handleRoot() {
  String html = "<html><body><h1>Prawn Farming Management System</h1></body><html>";
  server.send(200, "text/html", html);
}

void handleGETRequest(String call) {
  String ph_string = "";

  if(WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    ph_string =  String(ph_value, 2);
    Serial.println(ph_string);
    String serverPath = serverName + call + "?ph=" + ph_string;

    http.begin(client, serverPath.c_str());

    int httpResponseCode = http.GET();

    if(httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();

    server.send(200, "text/plain", ph_string);

  } else {
    Serial.println("WiFi disconnected");
  }
}

void serialComToArduino(String Data){
  s.println(Data);
  delay(3000);
}

void handleCheckPH() {
  handleGETRequest("check-ph");
}

void handleLowPHTreatment() {
  handleGETRequest("record-ph");
  // TODO: treatment functions
  // Serial.println("Low range Start!!");
  // serialComToArduino("lowPH");
  Serial.print(1);
}

void handleHighPHTreatment() {
  handleGETRequest("record-ph");
  // TODO: treatment functions
  // Serial.println("High range Start!!");
  // serialComToArduino("highPH");
  Serial.print(2);
}

// water level sensor function
bool waterLevel(int pin){
  if(digitalRead(pin)== LOW){
    return true;
  }
  else{
    return false;
  }
}


void handleWaterIOSystem(){
  // TODO: Treatment functions
  Serial.println("WaterIO System start!!");
  if(!waterLevel(D1)){
    Serial.println("in if");
    while(waterLevel(D1)== false) {
      digitalWrite(D5, LOW);
      Serial.println("Water in start");
    }
    digitalWrite(D5, HIGH);
    Serial.println("Water in stop");
  }else{
    Serial.println("Tank is full");
  }
}

void handleHarvestingSystem(){
  // TODO: Treatment functions
  Serial.println("Harvesting System start!!");
  if(waterLevel(D2) == true){
    while(waterLevel(D2) == true){
      digitalWrite(D6, LOW);
      digitalWrite(D7, LOW);
      Serial.println("solinoid and pump in start");
    }
    digitalWrite(D6, HIGH);
    digitalWrite(D7, HIGH);
    Serial.println("solinoid and pump in stop");
  }else{
    Serial.println("Tank is Empty");
  }
}

void handleNH3Treatment() {
  // TODO: Treatment functions
  // Serial.println("NH3 treatment start!!");
  // serialComToArduino("NH3Treat");
  if(!waterLevel(D3)){
    // Serial.println("in if");
    while(waterLevel(D3)== false) {
      digitalWrite(D4, LOW);
      Serial.println("Water in start");
    }
    digitalWrite(D4, HIGH);
    Serial.println("Water in stop");
  }else{
    Serial.println("Tank is full");
  }

  Serial.print(3);
  // Serial.println("NH3 treatment end!!");
}

void readNH3Value() {
  // nh3,1.23 <value>
  String inputString = Serial.readString();  // Read the input string from serial
  char charArray[inputString.length() + 1];  // Create a character array to hold the string

  // Copy the string from the String object to the character array
  inputString.toCharArray(charArray, sizeof(charArray));

  char *token;

  token = strtok(charArray, ",");
  if(token == "nh3") {
    token = strtok(NULL, ",");
    nh3_value = atof(token);
  }

  handleNH3Treatment();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // s.begin(9600);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  Serial.println("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  server.enableCORS(true);
  server.on("/", handleRoot);
  server.on("/checkPH", handleCheckPH);
  server.on("/startPHTreatLow", handleLowPHTreatment);
  server.on("/startPHTreatHigh", handleHighPHTreatment);
  server.on("/startNH3Treat", handleNH3Treatment);
  server.on("/startWaterIOSystem", handleWaterIOSystem);
  server.on("/startHarvestingSystem", handleHarvestingSystem);
  server.begin();

  //Sensor Pins
  pinMode(D1,INPUT_PULLUP); // Top WL Sensor
  pinMode(D2,INPUT_PULLUP); // Bottom WL Sensor
  pinMode(D3,INPUT_PULLUP); // Small Tank WL Sensor
  pinMode(D4, OUTPUT); // Large tank solinoid valve
  pinMode(D5, OUTPUT); // Small tank solinoid valve
  pinMode(D6, OUTPUT); // Large tank water out solinoid valve
  pinMode(D7, OUTPUT); // Large tank water out water pump

  digitalWrite(D4, HIGH); //Trun off relay channel
  digitalWrite(D5, HIGH); //Trun off relay channel
  digitalWrite(D6, HIGH); //Trun off relay channel
  digitalWrite(D7, HIGH); //Trun off relay channel
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

  if(Serial.available()) {
    readNH3Value();
   }
}
