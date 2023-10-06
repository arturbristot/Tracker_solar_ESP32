
#include <ESP32Servo.h>
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WebSocketsServer.h>

// Define as informações do sensor
int sensor = 0;

const char *ssid = "PlacaSolarIOT";
const char *password = "Placasolar";
AsyncWebServer server(80);
WebSocketsServer webSocket(81); // Porta WebSocket

void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_TEXT) {
    Serial.printf("[%u] Text received: %s\n", num, payload);
  }
}


// horizontal servo
Servo horizontal;
int servoh = 90;

int servohLimitHigh = 180;
int servohLimitLow = 30;

Servo vertical;
int servov = 90;

int servovLimitHigh = 150;
int servovLimitLow = 0;

float potencia = 0.0;

int tr, tl, br, bl;
int dtime, tol;

int avt, avd, avl, avr;

int dvert, dhoriz;

// LDR pin connections
int ldrTR = 34; // LDR top right
int ldrTL = 35; // LDR top left
int ldrBR = 32; // LDR bottom right
int ldrBL = 33; // LDR bottom left

int pin_tensao = 36;

void setup() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Falha ao montar o sistema de arquivos SPIFFS.");
    return;
  }

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.begin(115200);
  Serial.print("Endereço IP do AP: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "application/javascript");
  });

  server.begin();

  webSocket.begin();
  webSocket.onEvent(handleWebSocketMessage);
    //rede

  Serial.begin(9600);
  // servo connections
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  horizontal.setPeriodHertz(50);    // standard 50 hz servo
  horizontal.attach(19, 1000, 2000);  
  vertical.setPeriodHertz(50);    // standard 50 hz servo
  vertical.attach(21, 1000, 2000); 
  // move servos
  horizontal.write(90);
  vertical.write(45);
}


void loop() {
  ler_potencia();
  ler_analogs();
  calculos();
  //printar();
  servo_vert();
  servo_horizontal();

  webSocket.loop();

  // Converte o valor do sensor em uma string
  String sensorData = String(potencia);

  // Envie os dados do sensor por WebSocket
  webSocket.broadcastTXT(sensorData);
}

void ler_potencia() {
  float tensao = analogRead(pin_tensao);
  tensao = (3.3 * tensao) / 4095.0;
  tensao = 7.8 * tensao;
  potencia = tensao * tensao / 330;
  Serial.print("Potencia: ");
  Serial.print(potencia);
  Serial.print(" W  Tensao: ");
  Serial.print(tensao);
  Serial.println(" ");
}


void ler_analogs () {
  tr = analogRead(ldrTR); // top right
  tl = analogRead(ldrTL); // top left
  br = analogRead(ldrBR); // bottom right
  bl = analogRead(ldrBL); // bottom left
}

void calculos() { 
  dtime = 500; // change for debugging only
  tol = 40;

  avt = (tl + tr) / 2; // average value top
  avd = (bl + br) / 2; // average value bottom
  avl = (tl + bl) / 2; // average value left
  avr = (tr + br) / 2; // average value right

  dvert = avt - avd;  // check the difference of up and down
  dhoriz = avl - avr; // check the difference of left and right
}

void printar() {
  // send data to the serial monitor if desired
  Serial.print(tl);
  Serial.print(" ");
  Serial.print(tr);
  Serial.print(" ");
  Serial.print(bl);
  Serial.print(" ");
  Serial.print(br);
  Serial.print("  ");
  Serial.print(avt);
  Serial.print(" ");
  Serial.print(avd);
  Serial.print(" ");
  Serial.print(avl);
  Serial.print(" ");
  Serial.print(avr);
  Serial.print("  ");
  Serial.print(dtime);
  Serial.print("   ");
  Serial.print(tol);
  Serial.print("  ");
  Serial.print(servov);
  Serial.print("   ");
  Serial.print(servoh);
  Serial.println(" ");
}

void servo_vert() {
  // Verifique se a diferença está dentro da tolerância, caso contrário, mude o ângulo vertical
  if (-1 * tol > dvert || dvert > tol) {
    if (avt > avd) {
      servov = servov - 2; // Decrementa em vez de incrementar (mais rápido)
      if (servov < servovLimitLow) {
        servov = servovLimitLow;
      }
    }
    else if (avt < avd) {
      servov = servov + 2; // Incrementa em vez de decrementar (mais rápido)
      if (servov > servovLimitHigh) {
        servov = servovLimitHigh;
      }
    }
    vertical.write(servov);
  }
}

void servo_horizontal() {
    // check if the difference is in the tolerance else change horizontal angle
  if (-1 * tol > dhoriz || dhoriz > tol) {
    if (avl > avr) {
      servoh = servoh - 2; // Decrementa em vez de incrementar (mais rápido)
      if (servoh < servohLimitLow) {
        servoh = servohLimitLow;
      }
    }
    else if (avl < avr) {
      servoh = servoh + 2; // Incrementa em vez de decrementar (mais rápido)
      if (servoh > servohLimitHigh) {
        servoh = servohLimitHigh;
      }
    }
    else if (avl == avr) {
      // nothing
    }
    horizontal.write(servoh);
  }
}
