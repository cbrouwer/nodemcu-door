#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "vars.h"

#define DOOR_PIN D3

const char* wifiHostname = "nodemcu-frontdoor";
const char* mqttTopicPrefix = "frontdoor/";
const int enabledRfid = 1;

int doorVal = 0;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {
  pinMode(DOOR_PIN, INPUT);
  Serial.begin(74880);
  setupWifi();
  mqttClient.setServer(mqttServer, 1883);
  if (enabledRfid) setupRfid();
}

void setupWifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifiSsid);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(wifiHostname);
  WiFi.begin(wifiSsid, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void publish2MQTT(char* topicPostfix, const char* payload) {
  char topic[100];

  strcpy( topic, mqttTopicPrefix );
  strcat( topic, topicPostfix ); 
  mqttClient.publish(topic, payload);
}

void checkDoor() {
  int newDoorVal = digitalRead(DOOR_PIN);
  if (newDoorVal != doorVal) {
    Serial.print("Door sensor change detected! ");
    Serial.print(newDoorVal);
    Serial.println();
    // state change!
    doorVal = newDoorVal;
    const char* state = newDoorVal == HIGH ? "open" : "closed";
    publish2MQTT("door", state);
  }
}

void loop() {

  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  checkDoor();

  if (enabledRfid) loopRfid();
}
