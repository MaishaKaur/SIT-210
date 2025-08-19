#include <Wire.h>
#include <BH1750.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>

#define WIFI_SSID "Maisha"
#define WIFI_PASSWORD "maishaing"
#define MQTT_SERVER "df50967215c74807987115a53bb0855d.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883
#define MQTT_TOPIC "sensor/light"
#define MQTT_USER "hivemq.webclient.1755548851012"
#define MQTT_PASSWORD "7y<MphvS5AW9,w#F;4Rz"

BH1750 lightMeter;
WiFiSSLClient wifiClient;
PubSubClient mqttClient(wifiClient);

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect("ArduinoNanoIoT_BH1750", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("Connected to MQTT Broker!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: "); Serial.println(WiFi.localIP());

  Wire.begin();
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23)) {
    Serial.println("BH1750 not found at 0x23, trying 0x5C...");
    if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x5C)) {
      Serial.println("Could not find BH1750 sensor!");
      while (1);
    }
  }
  Serial.println("BH1750 Initialized.");

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();
}

void loop() {
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  float lux = lightMeter.readLightLevel();
  String payload = "{\"light\":" + String(lux) + "}";
  if (mqttClient.publish(MQTT_TOPIC, payload.c_str())) {
    Serial.print("Published: "); Serial.println(payload);
  } else {
    Serial.println("Publish failed");
  }

  delay(6000); // Adjusted to 30 seconds
}