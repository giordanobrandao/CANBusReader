// --- Inclusão de bibliotecas ---
#include <wifi.h>
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
AsyncWebServer ws("/ws");

// --- Armazenamento dos dados do carro ---
int rpm = 0;
int speed = 0;
int coolantTemp = 0;
float engineLoad = 0.0;

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
void requestPID(byte pid){
  CAN_frame_t tx_frame;
  tx_frame.FIR.B.FF = CAN_frame_std; // Frame padrão (11 bits)
  tx_frame.MsgID = 0x7DF; // ID de requisição OBD2 padrão
  tx_frame.FIR.B.DLC = 8; // Tamanho dos dados (8 bytes)
  tx_frame.data.u8[0] = 0x02; // Número de bytes de dados a seguir
  tx_frame.data.u8[1] = 0x01; // Modo 01 (dados em tempo real)
  tx_frame.data.u8[2] = pid;  // O PID que estamos pedindo
  tx_frame.data.u8[3] = 0x00; // Padding
  tx_frame.data.u8[4] = 0x00;
  tx_frame.data.u8[5] = 0x00;
  tx_frame.data.u8[6] = 0x00;
  tx_frame.data.u8[7] = 0x00;
  
  CAN.sendFrame(tx_frame);
  Serial.printf("Requisitando PID: 0x%02X\n", pid);
}

void parseCANMessage() {
  CAN_frame_t rx_frame;

  // Verifica se há um pacote disponível
  if (CAN.readFrame(rx_frame)) {
    // A ECU responde com IDs de 0x7E8 a 0x7EF. 0x7E8 é a principal.
    if (rx_frame.MsgID >= 0x7E8 && rx_frame.MsgID <= 0x7EF) {
      // Verifica se é uma resposta para o modo 01 (byte 1 = 0x41)
      if (rx_frame.data.u8[1] == 0x41) {
        byte pid = rx_frame.data.u8[2];
        byte byteA = rx_frame.data.u8[3];
        byte byteB = rx_frame.data.u8[4];
        
        Serial.printf("Resposta recebida para o PID: 0x%02X\n", pid);

        switch (pid) {
          case 0x0C: // RPM
            rpm = ((byteA * 256) + byteB) / 4;
            break;
          case 0x0D: // Velocidade (km/h)
            speed = byteA;
            break;
          case 0x05: // Temperatura do líquido de arrefecimento
            coolantTemp = byteA - 40;
            break;
          case 0x04: // Carga do motor
            engineLoad = (byteA * 100.0) / 255.0;
            break;
        }
      }
    }
  }
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
    request->send_P(200, "text/html", handleIndex);
  });

  server.begin();
  Serial.println("Servidor Web Iniciado.");

  // Inicia o CAN Bus
  // A maioria dos carros usa 500kbps para diagnóstico OBD2
  CAN.setPins(CAN_RX_PIN, CAN_TX_PIN);
  if (CAN.begin(500000)) {
    Serial.println("Controlador CAN iniciado com sucesso!");
    // Configura um filtro para aceitar apenas as respostas da ECU
    CAN.setFilter(0x7E8, 0x7F8); // Aceita de 0x7E8 a 0x7EF
  } else {
    Serial.println("Falha ao iniciar o controlador CAN.");
    // Trava aqui se o CAN não iniciar
    while(1);
  }
}

void loop() {
  // Limpa clientes WebSocket desconectados
  ws.cleanupClients();

  // Tarefa 1: Enviar requisições de PIDs em intervalos regulares
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Pede o próximo PID da lista
    byte pid = pidsToRequest[currentPIDIndex];
    requestPID(pid);
    
    // Avança para o próximo PID para a próxima vez
    currentPIDIndex++;
    if (currentPIDIndex >= sizeof(pidsToRequest)) {
      currentPIDIndex = 0;
    }
  }

  // Tarefa 2: Verificar e processar mensagens CAN recebidas
  parseCANMessage();

  // Tarefa 3: Enviar os dados atualizados para a página web via WebSocket
  // Cria uma string no formato JSON
  String json = "{\"rpm\":" + String(rpm) +
                ",\"speed\":" + String(speed) +
                ",\"coolantTemp\":" + String(coolantTemp) +
                ",\"engineLoad\":" + String(engineLoad) + "}";
  
  // Envia a string para todos os clientes conectados
  ws.textAll(json);
}

// --- Preparação da página web ---
void handleIndex() {
  delay(100);
  String indexHtml = String(main_html);
  delay(100);
  indexHtml.replace("%MAIN_CSS%", main_css);
  delay(100);
  indexHtml.replace("%MAIN_JS%", main_js);

  unsigned int contentLength = indexHtml.length();

  server.setContentLength(contentLength);
  server.send(200, "text/html", "");
  server.sendContent(indexHtml);
}