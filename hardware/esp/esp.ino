
#include <WiFiManager.h>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include <SocketIOclient.h>

//===========================================
#define DEBUG
#define SERVER "192.168.94.104"           //check lai
#define PORT 3000                         //check lai
#define JSON_SIZE 1024                    //byte
#define INTERVAL_SEND_TO_SERVER 5 * 1000  //ms
#define clientID "anhDuy"
//===========================================
SocketIOclient socketIO;
ESP8266WiFiMulti wifiMulti;
os_timer_t myTimer;
//===========================================
float tem, hum, mq6, mq7, mq135, dust;
String test;
//===========================================
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Anh Duy");
  //------------------------------------------
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("ESP8288_needToConfig", "66668888");
  if (!res) {
    Serial.println("Failed to connect");
    Serial.println("Retry entering the wifi credentials");
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("Connected to Wi-Fi!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  //------------------------------------------
  socketIO.begin(SERVER, PORT, "/socket.io/?EIO=4");
  socketIO.onEvent(socketIOEvent);
  Serial.println("Connecting to server");
  //------------------------------------------
  os_timer_setfn(&myTimer, sendData, NULL);
  os_timer_arm(&myTimer, INTERVAL_SEND_TO_SERVER, true);
}
//===========================================

void loop() {
  socketIO.loop();
  while (Serial.available() > 0) {
    String test = Serial.readStringUntil('\n');
    Serial.println(test);
    int comma1 = test.indexOf(', ');
    int comma2 = test.indexOf(', ', comma1 + 1);
    int comma3 = test.indexOf(', ', comma2 + 1);
    int comma4 = test.indexOf(', ', comma3 + 1);
    int comma5 = test.indexOf(', ', comma4 + 1);
    tem = test.substring(0, comma1).toFloat();
    hum = test.substring(comma1 + 1, comma2).toFloat();
    mq6 = test.substring(comma2 + 1, comma3).toFloat();
    mq7 = test.substring(comma3 + 1, comma4).toFloat();
    mq135 = test.substring(comma4 + 1, comma5).toFloat();
    dust = test.substring(comma5 + 1).toFloat();
  }
}
//===============================================
#define USE_SERIAL Serial
void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length) {
  digitalWrite(LED_BUILTIN, HIGH);
  String text1 = (char *)payload;
  switch (type) {
    case sIOtype_DISCONNECT:
      USE_SERIAL.printf("[IOc] Disconnected!\n");
      break;
    case sIOtype_CONNECT:
      USE_SERIAL.printf("[IOc] Connected to url: %s\n", payload);
      // join default namespace (no auto join in Socket.IO V3)
      socketIO.send(sIOtype_CONNECT, "/");
      break;
    case sIOtype_EVENT:
      if (text1.startsWith("[\".....\"")) {
        USE_SERIAL.printf("[IOc] phone 1 number change to: %s\n", payload);
      }
      break;
    case sIOtype_ACK:
      USE_SERIAL.printf("[IOc] get ack: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_ERROR:
      USE_SERIAL.printf("[IOc] get error: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_EVENT:
      USE_SERIAL.printf("[IOc] get binary: %u\n", length);
      hexdump(payload, length);
      break;
    case sIOtype_BINARY_ACK:
      USE_SERIAL.printf("[IOc] get binary ack: %u\n", length);
      hexdump(payload, length);
      break;
  }
  digitalWrite(LED_BUILTIN, LOW);
}

//===========================================
/**
 * @brief gửi dữ liệu lên server  (được gọi bởi timer)
 * 
 */
void sendData(void *arg) {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  //----------------------------------------
  DynamicJsonDocument doc(JSON_SIZE);
  JsonArray array = doc.to<JsonArray>();
  // TODO: doi cai  nay thanh cai phu hop la duoc
  array.add("message");
  //----------------------------------------
  JsonObject data = array.createNestedObject();
  data["clientID"] = clientID;
  data["mq6"] = String(mq6);
  data["mq7"] = String(mq7);
  data["mq135"] = String(mq135);
  data["temp"] = String(tem);
  data["humi"] = String(hum);
  data["bui"] = String(dust);
  //----------------------------------------
  String output;
  serializeJson(doc, output);
  socketIO.sendEVENT(output);
#ifdef DEBUG
  Serial.println(output);
#endif  // DEBUG
  delay(20);
}
//===========================================