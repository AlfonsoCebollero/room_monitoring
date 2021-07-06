// Importación librerías
#include <Servo.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x74, 0xF2 };
EthernetClient client;

int    HTTP_PORT   = 80;
String HTTP_METHOD = "GET";
char   HOST_NAME[] = "20.105.152.11";
String PATH_NAME   = "/room_data/alfonso/60e387fdd9f7df118bdb73c6";
int counter = 50;

LiquidCrystal lcd(12, 11, 5, 6, 7, 8);          
Servo myServo;

// Asignación de pines a constantes
const int piezo = A0;
const int tempSensor = A1;
const int lightSensor = A2;
const int switchPin = 2;
const int greenLed = 4;
const int yellowLed = 3;

// Declaración de variables y constantes
int knockVal;
int switchVal;
int tempSensorVal;
int lightSensorVal;
String luminosity;

const int quietKnock = 80;
const int loudKnock = 400;
int sensorLow = 1023;
int sensorHigh = 0;

boolean locked = false;
int numberOfKnocks = 0;

boolean checkForKnock(int value){
  if(value > quietKnock && value < loudKnock){
    digitalWrite(yellowLed, HIGH);
    delay(50);
    digitalWrite(yellowLed, LOW);
    Serial.print("OK: ");
    Serial.println(value);
    return true;
  }
  else
  {
    Serial.print("Not OK: ");
    Serial.println(value);
    return false;
  }  
}

void updateRoomData(String query) {
  if(client.connect(HOST_NAME, HTTP_PORT)) {
    // if connected:
    Serial.println("Connected to server");
    // make a HTTP request:
    // send HTTP header
    client.println(HTTP_METHOD + " " + PATH_NAME + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println(); // end HTTP header

    // send HTTP body
    if(counter == 50){
      client.println(query);
      counter = 0;   
    }
    else{
      counter ++;
    }
  } else {// if not connected:
    Serial.println("connection failed");
  }
}


void setup() {
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to obtaining an IP address using DHCP");
    while(true);
  }
  digitalWrite(yellowLed, HIGH);
  while(millis() < 5000) {
    lightSensorVal = analogRead(lightSensor);
    if (lightSensorVal > sensorHigh) {
      sensorHigh = lightSensorVal;
    }
    if (lightSensorVal < sensorLow) {
      sensorLow = lightSensorVal;
    }
  }
  lcd.begin(16, 2);
  digitalWrite(yellowLed,LOW);
  myServo.attach(9);
  pinMode(greenLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(switchPin, INPUT);
  pinMode(piezo,INPUT);
  pinMode(tempSensor,INPUT);
  Serial.begin(9600);
  myServo.write(0);
  digitalWrite(greenLed,HIGH);
  Serial.println("Abierto");

}

void loop() {
  if (locked == false) {
    switchVal = digitalRead(switchPin);
    if(switchVal == HIGH){
      numberOfKnocks = 0;
      locked = true;
      digitalWrite(greenLed,LOW);
      myServo.write(90);
      Serial.print("Cerrado");
      delay(1000);
  }
 }
  if(locked == true){
    knockVal = analogRead(piezo);
    Serial.println(knockVal);
    if(numberOfKnocks < 3 && knockVal > 0){
      if(checkForKnock(knockVal) == true){
        numberOfKnocks++;
      }
      Serial.print(3-numberOfKnocks);
      Serial.println("Golpes para abrir");
    }
    if(numberOfKnocks >= 3){
      locked = false;
      myServo.write(0);
      delay(20);
      digitalWrite(greenLed,HIGH);
      Serial.println("Abierto");
    }
  }
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
  Serial.print("lectura: ");
  Serial.println(tempSensorVal);
  Serial.print("temperature: ");
  Serial.println(temperature);
  Serial.print(luminosity);
  
  Serial.print(sensorLow);
  Serial.print(sensorHigh);
  Serial.println(lightSensorVal);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperatura: ");
  lcd.setCursor(13, 0);
  lcd.print(int(temperature));
  lcd.setCursor(0, 1);
  lcd.print("Luz: ");
  lcd.setCursor(5, 1);
  lcd.print(luminosity);
  temperature = temperature - 15;

  String querystring = String("?Temperatura=") + String(temperature) + String("&Luminosidad=") + String(luminosity);
  updateRoomData(querystring);
 }
