#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <I2S.h>
#include <ArduinoOTA.h>

const char* ssid = "mynet";
const char* password = "Coffe@123";

IPAddress serverIP(192, 168, 1, 20); // Replace with your PC IP
const int port = 3333;

WiFiUDP udp;

int16_t buffer[512]; // 512 samples context

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname("esp8266-mic");
  ArduinoOTA.begin();

  udp.begin(port);

  // Initialize I2S
  // INMP441 wiring for ESP8266:
  // L/R -> GND (for Mono/Left channel)
  // SCK -> GPIO15 (D8)
  // WS  -> GPIO2 (D4)
  // SD  -> GPIO3 (RX)
  I2S.begin(I2S_PHILIPS_MODE, 8000, 16);
  Serial.println("I2S initialized. Streaming audio...");
}

void loop() {
  ArduinoOTA.handle();

  // Read samples from I2S.
  // I2S.read() typically takes a void* or char* to buffer, and size in bytes.
  int bytesRead = I2S.read((char*)buffer, sizeof(buffer));

  if (bytesRead > 0) {
    udp.beginPacket(serverIP, port);
    udp.write((uint8_t*)buffer, bytesRead);
    udp.endPacket();
  }
}
