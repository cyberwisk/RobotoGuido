/**********************************************************
    WiFi Robot Remote Control Mode
    Usando as bibliotecas:
    ESP8266WiFi na versão 1.0 na pasta do pj: ESP8266WiFi.zip
    ESP8266WebServer na versão 1.0 na pasta do pj: ESP8266WebServer.zip
    Drive na versão 1.0.0 na pasta do pj: Drive.zip
    Aurelio Monteiro Avanzi
    04/03/2023
 **********************************************************/ 
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Drive.h>
#include "index.h" //HTML do Controle interno sem uso do APP, não estou usando por problemas de performance, fique a vontade para resolver...

//Define L298N pin mappings
const int IN1 = D7; /* GPIO13(D7) -> IN3   */
const int IN2 = D6; /* GPIO12(D6) -> IN3   */
const int IN3 = D4; /* GPIO4(D4)  -> IN3   */
const int IN4 = D3; /* GPIO5(D3)  -> IN3   */
Drive drive(IN1, IN2, IN3, IN4);

const int buzPin = D2;  /* GPIO16(D2)  */
const int ledPin = D13; /* GPIO14(D2) set as LED pin */
const int wifiLedPin = D9;  /* LED ligado se NodeMCU conectado no WiFi em STA mode */

String command;
int SPEED = 50;  /* Variave responsavel pela velocidade dos motores min:50, max:1023. */

//Ping HC-SR04 ultrasonic distance sensor
#define trigPin D8  /* GPIO0(D8) pino de saida do Ultrassom*/
#define echoPin D10  /* GPIO15(D10)  pino de entrada do Ultrassom*/
long duration, distance;
int obstacle;

ESP8266WebServer server(80);      /* Cria um objeto servidor Web que escuta aa solicitações HTTP na porta 80 */

unsigned long previousMillis = 0;

/*Usando roteador modo STA*/
String sta_ssid = "Frajola";      // set Wifi networks you want to connect to Router
String sta_password = "dd34e56134";  // set password for Wifi networks

/* Usando o ESP como ponto de acesso procure a rede RobotoGuido seguido do id do ESp8266*/
//String sta_ssid = "";      // set Wifi networks you want to connect to Router
//String sta_password = "";  // set password for Wifi networks

void setup(){
  Serial.begin(115200);    // set up Serial library at 115200 bps
  Serial.println();
  Serial.println("*WiFi Robot Remote Control Mode*");
  Serial.println("--------------------------------------");

  pinMode(buzPin, OUTPUT);      // seta o buzzer pin como saida
  pinMode(ledPin, OUTPUT);      // seta o LED pin como saida
  pinMode(wifiLedPin, OUTPUT);  // seta o Wifi LED pin como saida
  pinMode(trigPin, OUTPUT);     // seta o trigger pin como saida
  pinMode(echoPin, INPUT);      // seta o echo pin como saida
  
  digitalWrite(buzPin, LOW);
  digitalWrite(ledPin, LOW);
  digitalWrite(wifiLedPin, HIGH);
  
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
    server.handleClient();        // escuta os comandos http dos clientes
      command = server.arg("State");          //HTPP request, com os argumentos passados no "State"
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
      else if (command == "H") BackwardLeft();
      else if (command == "I") ForwardRight();
      else if (command == "J") BackwardRight();
      else if (command == "S") Stop();
      else if (command == "V") BeepHorn();
      else if (command == "W") TurnLightOn();
      else if (command == "w") TurnLightOff();
      else if (command == "A") AutoRoteOn();
      else if (command == "a") AutoRoteOff();
}

// function prototypes for HTTP handlers
void HTTP_handleRoot(void){
  server.send(200, "text/html", "<h2>RobotoGuiodo OK </h2>");
  //server.send(200, "text/html", MAIN_page); //pagina index.h retirada por provocar atraso nos comandos.
  if( server.hasArg("State") ){
  Serial.println(server.arg("State"));
  }
}
void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
  drive.stopMoving();
}

// function to move forward
void Forward(){
  obstacle = ping();
  if (obstacle > 10){ //se não encontrou nehum obstaculo a menos de 10cm segue o barco...
    drive.moveForward(SPEED);
  }else{ //se não, para, pensa meio segundo, sorteia um lado, vira no eixo e só continua  na velocidade minima quando encontra caminho livre.
        drive.stopMoving();
        delay(500);
        tone(buzPin, 3000, 20);  
        int turn = random(2);
            while (obstacle < 10) {
                delay(100)
                if (turn = 1){
                drive.turnRight(50);
                obstacle = ping();
                }else{
                drive.turnLeft(50);
                obstacle = ping();
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

void Backward(){
  drive.moveBackward(SPEED);
  Serial.print("Backward "); 
  Serial.println(SPEED);
}

void TurnLeft(){
  drive.turnLeft(SPEED);
  Serial.print("TurnLeft "); 
  Serial.println(SPEED);
}

void TurnRight(){
  drive.turnRight(SPEED);
  Serial.print("TurnRight "); 
  Serial.println(SPEED);
}

void ForwardLeft(){
  drive.moveforwardLeft(SPEED);
}

void BackwardLeft(){
  drive.moveBackwardLeft(SPEED);
}

void ForwardRight(){
  drive.moveforwardRight(SPEED);
}

void BackwardRight(){
  drive.moveBackwardRight(SPEED);
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

/* Funcao de Autorote*/
void AutoRoteOn(){
  digitalWrite(ledPin, HIGH);
}
void AutoRoteOff(){
  digitalWrite(ledPin, LOW);
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
int ping(){
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
