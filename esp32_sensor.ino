#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 4          // DHT data pin connected to GPIO4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// WiFi settings
const char* ssid = "Your_WiFi_Name";
const char* password = "Your_WiFi_Password";

// AWS IoT MQTT broker
const char* mqtt_server = "your-aws-endpoint.amazonaws.com";  
const int mqtt_port = 1883;  
const char* mqtt_topic = "esp32/sensor/data";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  char payload[100];
  sprintf(payload, "{\"temperature\": %.2f, \"humidity\": %.2f}", t, h);
  Serial.println(payload);

  client.publish(mqtt_topic, payload);
  delay(5000); // send every 5 sec
}
