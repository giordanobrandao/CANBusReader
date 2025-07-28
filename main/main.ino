// --- Inclusão de bibliotecas ---
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <CAN.h>
#include "main_html.h"
#include "main_css.h"
#include "main_js.h"

// --- Ponto de Acesso ---
const char* ssid = "CanReceiver";
const char* passwd = "canreceiver";

// --- Pinos para o CAN Bus ---
#define CAN_TX_PIN 21
#define CAN_RX_PIN 22

// --- Objetos do Servidor ---
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// --- Armazenamento dos dados do carro ---
volatile int rpm = 0;
volatile int speed = 0;
volatile int coolantTemp = 0;
volatile float engineLoad = 0.0;
volatile bool newDataAvailable = false;

// --- Controle do tempo de requisição dos PIDs ---
unsigned long previousMillis = 0;
const long interval = 200;
int currentPIDIndex = 0;

// --- Lista de PIDs a serem lidos ---
byte pidsToRequest[] = {0x0c, 0x0D, 0x05, 0x04};

// --- Tratamento de eventos do WebSocket e sua inicialização ---
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  switch (type)  {
  case WS_EVT_CONNECT:
    Serial.printf("Cliente WebSocket #%u conectado de %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("Cliente WebSocket #%u desconectado\n", client->id());
    break;
  case WS_EVT_DATA:
    /* Do Something */
    break;
  
  default:
    break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

// --- Funções do CAN Bus ---
void onReceive(int packetSize) {
  // Verifica se é uma resposta OBD2 (ID 0x7E8 a 0x7EF)
  if (CAN.packetId() >= 0x7E8 && CAN.packetId() <= 0x7EF) {
    // Verifica se é uma resposta para o modo 01 (byte 1 = 0x41)
    if (CAN.read() == 0x02 && CAN.read() == 0x41) {
      byte pid = CAN.read();
      byte byteA = CAN.read();
      byte byteB = CAN.read();
      
      switch (pid) {
        case 0x0C: rpm = ((byteA * 256) + byteB) / 4; break;
        case 0x0D: speed = byteA; break;
        case 0x05: coolantTemp = byteA - 40; break;
        case 0x04: engineLoad = (byteA * 100.0) / 255.0; break;
      }
      newDataAvailable = true;
    }
  }
}

void requestPID(byte pid) {
  Serial.printf("Requisitando PID: 0x%02X\n", pid);
  CAN.beginPacket(0x7DF, 8); // ID de requisição OBD2, 8 bytes de dados
  CAN.write(0x02); // Número de bytes de dados a seguir
  CAN.write(0x01); // Modo 01 (dados em tempo real)
  CAN.write(pid);  // O PID que estamos pedindo
  // O restante é preenchido com 0 pela biblioteca
  CAN.endPacket();
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando Leitor OBD2...");

  // Inicia Wi-Fi em modo ponto de acesso
  WiFi.softAP(ssid, passwd);
  Serial.println("Ponto de acesso criado. IP: ");
  Serial.println(WiFi.softAPIP());

  initWebSocket();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String indexHtml = String(main_html);
    indexHtml.replace("%MAIN_CSS%", main_css);
    indexHtml.replace("%MAIN_JS%", main_js);

    request->send(200, "text/html", indexHtml);
  });

  server.begin();
  Serial.println("Servidor Web Iniciado.");

  // Inicia o CAN Bus
  CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);
  if (!CAN.begin(500E3)) {
    Serial.println("Falha ao iniciar controlador CAN!");
  }
  Serial.println("Controlador CAN iniciado com sucesso!");
    
  CAN.onReceive(onReceive);
}

void loop() {
  // Limpa clientes WebSocket desconectados
  ws.cleanupClients();

  // Enviar requisições de PIDs em intervalos regulares
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Pede o próximo PID da lista
    byte pid = pidsToRequest[currentPIDIndex];
    requestPID(pid);
    
    // Avança para o próximo PID para a próxima vez
    currentPIDIndex = (currentPIDIndex + 1) % (sizeof(pidsToRequest));
  }

  // Enviar os dados atualizados para a página web via WebSocket
  if (newDataAvailable){
    String json = "{\"rpm\":" + String(rpm) +
                  ",\"speed\":" + String(speed) +
                  ",\"coolantTemp\":" + String(coolantTemp) +
                  ",\"engineLoad\":" + String(engineLoad) + "}";
    
    // Envia a string para todos os clientes conectados
    ws.textAll(json);
    newDataAvailable = false;
  }
}