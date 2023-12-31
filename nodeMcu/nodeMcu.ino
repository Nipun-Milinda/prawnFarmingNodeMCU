#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>

SoftwareSerial s(D9, D10);

const char* ssid = "South Pole";
const char* password = "214110Gbdnm";

String serverName = "http://192.168.1.154:5000/api/";

ESP8266WebServer server(80);

double ph_value = 6.4;
double nh3_value = 1.234;

char bioChipStatus;
char slakelimeStatus;
char sugarStatus;

void keypadHandler(char data) {
  if (data == 'a') {
    handleWaterIOSystem();
  } else if (data == 'b') {
    handleHarvestingSystem();
  } else if (data == 'c') {

    if (ph_value < 7.5) {
      Serial.print(5);
      delay(3000);
      if(slakelimeStatus == '1'){
        smallTankInput(1);
      }
    }else if(ph_value > 8.5){
      Serial.print(6);
      delay(3000);
      if(sugarStatus == '1'){
        smallTankInput(2);
      }
    }else if(ph_value>7.5 && ph_value<8.5){
      Serial.print(7);
    }
  } else if (data == 'd') {
    handleNH3Treatment();
  }
}

void webHandler(char data) {
  if (data == '1') {
    char bioChipStatus = '1';
  } else if (data == '2') {
    char bioChipStatus = '0';
  } else if (data == '3') {
    char slakelimeStatus = '1';
  } else if (data == '4') {
    char slakelimeStatus = '0';
  } else if (data == '5') {
    char sugarStatus = '1';
  } else if (data == '6') {
    char sugarStatus = '0';
  }
}


void handleSerialData(char data) {

  if (data == '1' || data == '2' || data == '3' || data == '4' || data == '5' || data == '6') {
    keypadHandler(data);
  } else if (data == 'a' || data == 'b' || data == 'c' || data == 'd') {
    webHandler(data);
  }

  
}

void handleRoot() {
  String html = "<html><body><h1>Prawn Farming Management System</h1></body><html>";
  server.send(200, "text/html", html);
}

void handleGETRequest(String call) {
  String valueTxt = "";
  String serverPath = "";

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    if(call=="check-ph") {
      //ph value
      valueTxt =  String(ph_value, 2);
      // Serial.println(ph_string);
      serverPath = serverName + call + "?ph=" + valueTxt;
    } else if(call=="biochip-status") {
      //bioChipStatus
      valueTxt =  String(bioChipStatus);
      // Serial.println(bioChipStatus_string);
      serverPath = serverName + call + "?biochip=" + valueTxt;
    } else if(call == "slakelime-status") {
      //slakelime
      valueTxt =  String(slakelimeStatus);
      // Serial.println(slakelimeStatus_string);
      serverPath = serverName + call + "?slakelime=" + valueTxt;
    } else if(call == "sugar-status") {
      //sugar
      valueTxt =  String(sugarStatus);
      // Serial.println(sugarStatus_string);
      serverPath = serverName + call + "?sugar=" + valueTxt;
    }

    http.begin(client, serverPath.c_str());

    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();

    server.send(200, "text/plain", valueTxt);

  } else {
    Serial.println("WiFi disconnected");
  }
}

void serialComToArduino(String Data) {
  s.println(Data);
  delay(3000);
}

void handleBioChipStatus() {
  handleGETRequest("biochip-status");
  Serial.print(4);
}

void handleSlakelimeStatus() {
  handleGETRequest("slakelime-status");
  Serial.print(5);
}

void handleSugarStatus() {
  handleGETRequest("sugar-status");
  Serial.print(6);
}

void handleCheckPH() {
  handleGETRequest("check-ph");
}

void handleLowPHTreatment() {
  handleGETRequest("record-ph");
  // TODO: treatment functions
  Serial.print("Low start");
  Serial.print(1);
}

void handleHighPHTreatment() {
  handleGETRequest("record-ph");
  // TODO: treatment functions
  Serial.print(2);
}

// water level sensor function
bool waterLevel(int pin) {
  if (digitalRead(pin) == LOW) {
    return true;
  } else {
    return false;
  }
}


void handleWaterIOSystem() {
  // TODO: Treatment functions
  Serial.println("WaterIO System start!!");
  if (!waterLevel(D2)) {
    Serial.println("in if");
    while (waterLevel(D2) == false) {
      digitalWrite(D6, LOW);
      Serial.println("Water in start");
    }
    digitalWrite(D6, HIGH);
    Serial.println("Water in stop");
  } else {
    Serial.println("Tank is full");
  }
}

void handleHarvestingSystem() {
  // TODO: Treatment functions
  Serial.println("Harvesting System start!!");
  if (waterLevel(D1) == true) {
    while (waterLevel(D1) == true) {
      digitalWrite(D4, LOW);
      digitalWrite(D5, LOW);
      Serial.println("solinoid and pump in start");
    }
    digitalWrite(D4, HIGH);
    digitalWrite(D5, HIGH);
    Serial.println("solinoid and pump in stop");
  } else {
    Serial.println("Tank is Empty");
  }
}

//Need to fix
void handleNH3Treatment() {
  // TODO: Treatment functions
  Serial.print(3);
  if (!waterLevel(D3)) {
    while (waterLevel(D3) == false) {
      digitalWrite(D4, LOW);
      Serial.println("Water in start");
    }
    digitalWrite(D4, HIGH);
    Serial.println("Water in stop");
  } else {
    Serial.println("Tank is full");
  }
}

void readNH3Value() {
  // nh3,1.23 <value>
  String inputString = Serial.readString();  // Read the input string from serial
  char charArray[inputString.length() + 1];  // Create a character array to hold the string

  // Copy the string from the String object to the character array
  inputString.toCharArray(charArray, sizeof(charArray));

  char* token;

  token = strtok(charArray, ",");
  if (token == "nh3") {
    token = strtok(NULL, ",");
    nh3_value = atof(token);
  }

  handleNH3Treatment();
}

// double calculate_pH()
// {
//   pH_Value = analogRead(A0);
//   Voltage = pH_Value * (5.0 / 1023.0);
//   double final = 11.07 * Voltage - 33.475;
//   return final;
// }


void smallTankInput(int data){
  if (!waterLevel(D3)) {
    // Serial.println("in if");
    while (waterLevel(D3) == false) {
      digitalWrite(D7, LOW);
      // Serial.println("Water in start");
    }
    digitalWrite(D7, HIGH);
    // Serial.println("Water in stop");
    Serial.print(data);
  } else {
    // Serial.println("Tank is full");
    Serial.print(data);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // s.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("");
  Serial.println("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  server.enableCORS(true);
  server.on("/", handleRoot);
  server.on("/checkPH", handleCheckPH);
  server.on("/bioChipStatus", handleBioChipStatus);
  server.on("/slakelimeStatus", handleSlakelimeStatus);
  server.on("/sugarStatus", handleSugarStatus);
  server.on("/startPHTreatLow", handleLowPHTreatment);
  server.on("/startPHTreatHigh", handleHighPHTreatment);
  server.on("/startNH3Treat", handleNH3Treatment);
  server.on("/startWaterIOSystem", handleWaterIOSystem);
  server.on("/startHarvestingSystem", handleHarvestingSystem);
  server.begin();

  //Sensor Pins
  pinMode(D1, INPUT_PULLUP);  // Top WL Sensor
  pinMode(D2, INPUT_PULLUP);  // Bottom WL Sensor
  pinMode(D3, INPUT_PULLUP);  // Small Tank WL Sensor
  pinMode(D4, OUTPUT);        // Large tank solinoid valve
  pinMode(D5, OUTPUT);        // Small tank solinoid valve
  pinMode(D6, OUTPUT);        // Large tank water out solinoid valve
  pinMode(D7, OUTPUT);        // Large tank water out water pump

  digitalWrite(D4, HIGH);  //Trun off relay channel
  digitalWrite(D5, HIGH);  //Trun off relay channel
  digitalWrite(D6, HIGH);  //Trun off relay channel
  digitalWrite(D7, HIGH);  //Trun off relay channel

  // pinMode(pH_Value, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

  if (Serial.available()) {
    char command = Serial.read();
    handleSerialData(command);

    // readNH3Value();
  }
}
