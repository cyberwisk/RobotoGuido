/**********************************************************
    WiFi Robot Remote Control Mode
    Usando a biblioteca ESP8266WiFi na versão 1.0 na pasta: ESP8266WiFi.zip
    Usando a biblioteca ESP8266WebServer na versão 1.0 na pasta: ESP8266WebServer.zip
    Usando a biblioteca Drive na versão 1.0.0 na pasta: Drive.zip
    Aurelio Monteiro Avanzi
    04/03/2023
 **********************************************************/ 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Drive.h>
#include "index.h" //HTML do Controle

//Define L298N pin mappings
const int IN1 = D7;
const int IN2 = D6;
const int IN3 = D4;
const int IN4 = D3;
Drive drive(IN1, IN2, IN3, IN4);

const int buzPin = D2;
const int ledPin = D13;      // D15/D3 set digital pin D8 as LED pin (use super bright LED) 
const int wifiLedPin = D9;  // set digital pin D0 as indication, the LED turn on if NodeMCU connected to WiFi as STA mode

String command;
int SPEED = 50;        // 50 - 1023.

//Ping HC-SR04 ultrasonic distance sensor
#define trigPin D8
#define echoPin D10
long duration, distance;
int obstacle;

ESP8266WebServer server(80);      // Create a webserver object that listens for HTTP request on port 80

unsigned long previousMillis = 0;

String sta_ssid = "Frajola";      // set Wifi networks you want to connect to Router
String sta_password = "dd34e56134";  // set password for Wifi networks

//String sta_ssid = "";      // set Wifi networks you want to connect to Router
//String sta_password = "";  // set password for Wifi networks

void setup(){
  Serial.begin(115200);    // set up Serial library at 115200 bps
  Serial.println();
  Serial.println("*WiFi Robot Remote Control Mode*");
  Serial.println("--------------------------------------");

  pinMode(buzPin, OUTPUT);      // sets the buzzer pin as an Output
  pinMode(ledPin, OUTPUT);      // sets the LED pin as an Output
  pinMode(wifiLedPin, OUTPUT);  // sets the Wifi LED pin as an Output
  digitalWrite(buzPin, LOW);
  digitalWrite(ledPin, LOW);
  digitalWrite(wifiLedPin, HIGH);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);   
  
  // set NodeMCU Wifi hostname based on chip mac address
  String chip_id = String(ESP.getChipId(), HEX);
  int i = chip_id.length()-4;
  chip_id = chip_id.substring(i);
  chip_id = "RobotoGuido-Chip_id: " + chip_id;
  String hostname(chip_id);
 
  Serial.println();
  Serial.println("Hostname: "+hostname);

  // first, set NodeMCU as STA mode to connect with a Wifi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
  Serial.println("");
  Serial.print("Connecting to: ");
  Serial.println(sta_ssid);
  Serial.print("Password: ");
  Serial.println(sta_password);

  // try to connect with Wifi network about 10 seconds
  unsigned long currentMillis = millis();
  previousMillis = currentMillis;
  while (WiFi.status() != WL_CONNECTED && currentMillis - previousMillis <= 10000) {
    delay(500);
    Serial.print(".");
    currentMillis = millis();
  }

  // if failed to connect with Wifi network set NodeMCU as AP mode
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("*WiFi-STA-Mode*");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(wifiLedPin, HIGH);    // Wifi LED off when connected to Wifi as STA mode
    delay(2000);
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostname.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.println("");
    Serial.println("WiFi failed connected to " + sta_ssid);
    Serial.println("");
    Serial.println("*WiFi-AP-Mode*");
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    digitalWrite(wifiLedPin, LOW);   // Wifi LED on when status as AP mode
    delay(2000);
  }

  server.on ( "/", HTTP_handleRoot );       // call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound ( HTTP_handleRoot );    // when a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.begin();                           // actually start the server
}


void loop() {
    server.handleClient();        // listen for HTTP requests from clients
      command = server.arg("State");          // check HTPP request, if has arguments "State" then saved the value
      if (command == "0") SPEED = 0;
      else if (command == "1") SPEED = 55;
      else if (command == "2") SPEED = 70;
      else if (command == "3") SPEED = 90;
      else if (command == "4") SPEED = 200;
      else if (command == "5") SPEED = 400;
      else if (command == "6") SPEED = 600;
      else if (command == "7") SPEED = 700;
      else if (command == "8") SPEED = 800;
      else if (command == "9") SPEED = 900;
      else if (command == "q") SPEED = 1023;
      else if (command == "F") Forward();
      else if (command == "B") Backward();
      else if (command == "R") TurnRight();
      else if (command == "L") TurnLeft();
      else if (command == "G") ForwardLeft();
      else if (command == "I") ForwardRight();
      else if (command == "S") Stop();
      else if (command == "V") BeepHorn();
      else if (command == "W") TurnLightOn();
      else if (command == "w") TurnLightOff();
}

// function prototypes for HTTP handlers
void HTTP_handleRoot(void){
  server.send(200, "text/html", "<h2>RobotoGuiodo OK </h2>"); //Send web page in index.h
  //server.send(200, "text/html", MAIN_page); //Send web page in index.h
  if( server.hasArg("State") ){
  Serial.println(server.arg("State"));
  }
}
void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}

// function to move forward
void Forward(){
  obstacle = ping(0);
  if (obstacle > 10){ //se não encontrou nehum obstaculo a menos de 10cm segue o bacro...
    drive.moveForward(SPEED);
  }
  else{
    drive.stopMoving();
    delay(500);
    tone(buzPin, 3000, 20);  
    int turn = random(2);
      while (obstacle < 10) {
      obstacle = ping(0);
        if (turn = 1){
        drive.turnRight(50);
        }
        else{
        drive.turnLeft(50);
        turn = 1;      
        }
      }
    drive.moveForward(50);
  }  
    Serial.print("Forward "); 
    Serial.println(SPEED);
    Serial.print("Distancia ");
    Serial.println(obstacle);
}

// function to move backward
void Backward(){
  drive.moveBackward(SPEED);
  Serial.print("Backward "); 
  Serial.println(SPEED);
}

// function to turn right
void TurnRight(){
  drive.turnRight(SPEED);
  Serial.print("TurnRight "); 
  Serial.println(SPEED);
}

// function to turn left
void TurnLeft(){
  drive.turnLeft(SPEED);
  Serial.print("TurnLeft "); 
  Serial.println(SPEED);
}

void ForwardLeft(){
  drive.moveforwardLeft(SPEED);
}

// function to move forward right
void ForwardRight(){
  drive.moveforwardRight(SPEED);
}

// function to stop motors
void Stop(){ 
drive.stopMoving();
}

// function to beep a buzzer
void BeepHorn(){
  int var = 0;
   while (var < 3) {
      int k = random(3000,1000);
    for (int i = 0; i <=  random(100,2000); i++){
        tone(buzPin, k+(-i*2));          
        delay(random(.9,2));             
    } 
    for (int i = 0; i <= random(100,1000); i++){
        tone(buzPin, k + (i * 10));          
        delay(random(.9,2));             
    }
    tone (buzPin,100,100); 
  var++;
}
}

// function to turn on LED
void TurnLightOn(){
  digitalWrite(ledPin, HIGH);
}

// function to turn off LED
void TurnLightOff(){
  digitalWrite(ledPin, LOW);
}

// HC-SR04 ultrasonic distance sensor
// A velocidade do som e de 340 m/s ou 29 microssegundos por centimetro.
// O ping e enviado para frente e reflete no objeto para encontrar a distancia
// A distancia do objeto fica na metade da distancia percorrida.
int ping(int mode){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); //Microsegundos
  distance = (duration/2) / 29.1; //Centimetros
  delay(100);
  return distance;
} // END Ping
