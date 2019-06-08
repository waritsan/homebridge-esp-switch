#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WifiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const int pin = D1;
WiFiClient espClient;
WiFiManager wifiManager;
PubSubClient client(espClient);
IPAddress server(192, 168, 1, 48);
char mName[16];
const char* mServiceName = "Light";
const char* mService = "Switch";
const char* mCharacteristic = "On";
void printLog(const char* topic, const char* payload) {
  Serial.print("[");
  Serial.print(topic);
  Serial.print("] ");
}

void addAccessory() {
  StaticJsonDocument<128> doc;
  doc["name"] = mName;
  doc["service_name"] = mServiceName;
  doc["service"] = mService;
  char payload[128];
  size_t n = serializeJson(doc, payload);
  const char* topic = "homebridge/to/add";
  client.publish(topic, payload, n);
  Serial.printf("Message sent [%s] %s\n", topic, payload);
}

void setValue(const char* serviceName, const char* characteristic, const int value) {
  if (strcmp(serviceName, mServiceName) == 0) {
    digitalWrite(pin, value);
  }
}

void processMessage(char* topic, byte* payload) {
  if (strcmp(topic, "homebridge/from/response") == 0) {
    StaticJsonDocument<512> doc;
    deserializeJson(doc, payload);
    if (doc.containsKey("ack")) return addAccessory();
    if (doc.containsKey(mName)) {
      JsonObject serviceNames = doc[mName]["characteristics"].as<JsonObject>();
      for (JsonPair serviceNameKeyValue: serviceNames) {
        const char* serviceName = serviceNameKeyValue.key().c_str();
        JsonObject characteristics = serviceNameKeyValue.value().as<JsonObject>();
        for (JsonPair characteristicKeyValue: characteristics) {
          const char* characteristic = characteristicKeyValue.key().c_str();
          const int value = characteristicKeyValue.value().as<int>();
          setValue(serviceName, characteristic, value);
        }
      }
    }
  } else if (strcmp(topic, "homebridge/from/set") == 0) {
    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload);
    const char* name = doc["name"];
    if (strcmp(name, mName) == 0) {
      const char* serviceName = doc["service_name"];
      const char* characteristic = doc["characteristic"];
      const int value = doc["value"];
      setValue(serviceName, characteristic, value);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  processMessage(topic, payload);
}

void connect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      client.subscribe("homebridge/from/response");
      client.subscribe("homebridge/from/set");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void getAccessory() {
  StaticJsonDocument<64> doc;
  doc["name"] = mName;
  char payload[64];
  size_t n = serializeJson(doc, payload);
  const char* topic = "homebridge/to/get";
  client.publish(topic, payload, n);
  Serial.printf("Message sent [%s] %s\n", topic, payload);
}

void setup() {
  Serial.begin(9600);
  pinMode(pin, OUTPUT);
  itoa(ESP.getChipId(), mName, 10);
  wifiManager.autoConnect();
  client.setServer(server, 1883);
  client.setCallback(callback);
  connect();
  getAccessory();
}

void loop() {
  if (!client.connected()) {
    connect();
  }
  client.loop();
}