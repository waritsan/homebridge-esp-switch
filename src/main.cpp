#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WifiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

WiFiClient espClient;
WiFiManager wifiManager;
PubSubClient client(espClient);
IPAddress server(192, 168, 1, 48);
char sensorName[64];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  DynamicJsonDocument doc(128);
  deserializeJson(doc, payload);
  const char* name = doc["name"];
  Serial.println(sensorName);
  if (strcmp(sensorName, name) == 0) {
    Serial.println(name);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // client.publish("outTopic","hello world");
      // ... and resubscribe
      client.subscribe("homebridge/from/get");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  itoa(ESP.getChipId(), sensorName, 10);
  wifiManager.autoConnect();
  client.setServer(server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}