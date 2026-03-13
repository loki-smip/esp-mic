#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <I2S.h>
#include <ArduinoOTA.h>

// --- Configuration ---
const char* STASSID = "your-ap";
const char* STAPSK  = "your_password";
const IPAddress listener(192, 168, 1, **); // Your PC IP
const int port = 3333;

WiFiUDP udp;

#define BUFFER_SIZE 256
int16_t audioBuffer[BUFFER_SIZE];

void setup() {
  system_update_cpu_freq(160); 
  
  Serial.begin(115200);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  ArduinoOTA.setHostname("esp8266-mic");
  ArduinoOTA.begin();

  i2s_rxtx_begin(true, false); 
  i2s_set_rate(16000);

  udp.begin(port);
  Serial.println("\nStreaming Started...");
}

void loop() {
  ArduinoOTA.handle();

  int16_t left, right;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    i2s_read_sample(&left, &right, true); 
    audioBuffer[i] = left; 
  }

  udp.beginPacket(listener, port);
  udp.write((uint8_t*)audioBuffer, sizeof(audioBuffer));
  udp.endPacket();
}
