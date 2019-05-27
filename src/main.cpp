#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WifiManager.h>
#include <PubSubClient.h>

WiFiClient espClient;
WiFiManager wifiManager;
PubSubClient client(espClient);

void setup() {
  wifiManager.autoConnect();
}

void loop() {
  // put your main code here, to run repeatedly:
}