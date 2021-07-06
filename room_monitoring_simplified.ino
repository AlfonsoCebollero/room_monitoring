// Importación librerías
#include <Servo.h>
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x74, 0xF2 };
EthernetClient client;

int    HTTP_PORT   = 80;
String HTTP_METHOD = "POST";
char   HOST_NAME[] = "20.105.152.11";
String PATH_NAME   = "/room_data/alfonso/60e387fdd9f7df118bdb73c6";
int counter = 50;
String get_response;
       
Servo myServo;

// Asignación de pines a constantes
const int tempSensor = A1;
const int lightSensor = A2;

// Declaración de variables y constantes
int tempSensorVal;
int lightSensorVal;
String luminosity;

int sensorLow = 1023;
int sensorHigh = 0;

char respuesta[200];

boolean locked = false;

char updateLocker(){
    if(client.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    if(counter == 50){
      client.println(String("GET") + " " + PATH_NAME + " HTTP/1.1");
      client.println("Host: " + String(HOST_NAME));
      client.println("Connection: close");
      client.println(); // end HTTP header

    int index = 0;
    while(client.connected()) {
      if(client.available()){
        // read an incoming byte from the server and print it to serial monitor:
        if(index < 199){
          respuesta[index] = client.read();
          Serial.print(respuesta);
        }
      }
      else{
        return respuesta;
      }
    }
    
    }
  }
    
    // send HTTP body
    else {// if not connected:
    Serial.println("connection failed");
    return respuesta;
  }
}

void updateRoomData(String query) {
  if(client.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    if(counter == 50){
      client.println(HTTP_METHOD + " " + PATH_NAME + query + " HTTP/1.1");
      client.println("Host: " + String(HOST_NAME));
      client.println("Connection: close");
      client.println(); // end HTTP header

    while(client.connected()) {
      if(client.available()){
        // read an incoming byte from the server and print it to serial monitor:
        char c = client.read();
        Serial.print(c);
      }
    }
    }
    else{
      counter ++;
    }
  }
    // send HTTP body
    else {// if not connected:
    Serial.println("connection failed");
  }
    
  } 

void setup() {
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to obtaining an IP address using DHCP");
    while(true);
  }
  while(millis() < 5000) {
    lightSensorVal = analogRead(lightSensor);
    if (lightSensorVal > sensorHigh) {
      sensorHigh = lightSensorVal;
    }
    if (lightSensorVal < sensorLow) {
      sensorLow = lightSensorVal;
    }
  }
  myServo.attach(9);
  pinMode(tempSensor,INPUT);
  Serial.begin(9600);
  myServo.write(0);
  Serial.println("Abierto");

}

void loop() {
  lightSensorVal = analogRead(lightSensor);
  if (lightSensorVal < sensorLow){
    luminosity = "Low";
  }
  else if (lightSensorVal > sensorHigh) {
    luminosity = "High";
  }
  else {
    luminosity = "Medium";
  }
  tempSensorVal = analogRead(tempSensor);
  float voltage = (tempSensorVal/1024.0) * 5.0;
  float temperature = (voltage - 0.5) * 100;

  String querystring = String("?Temperatura=") + String(temperature) + String("&Luminosidad=") + String(luminosity);
  Serial.println(querystring);
  updateRoomData(querystring);
 }
