#include <SPI.h>
#include <WiFiNINA.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22

char ssid[] = "1313";
char pass[] = "1234567a";

String apiKey = "DCDDKSPB88JM2K3V";
const char* server = "api.thingspeak.com";

WiFiClient client;
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastTime = 0;
unsigned long interval = 16000;

void setup() {
  Serial.begin(9600);
  dht.begin();

  Serial.print("Connecting to WiFi");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  if (millis() - lastTime > interval) {
    float temperature = dht.readTemperature();

    if (isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    if (client.connect(server, 80)) {
      String postData = "api_key=" + apiKey + "&field1=" + String(temperature);

      client.println("POST /update HTTP/1.1");
      client.println("Host: api.thingspeak.com");
      client.println("Connection: close");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(postData.length());
      client.println();
      client.println(postData);

      Serial.println("Data sent to ThingSpeak!");

      while (client.connected() && !client.available()) delay(1);
      while (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println("ThingSpeak response: " + line);
      }

    } else {
      Serial.println("Connection to ThingSpeak failed.");
    }

    client.stop();
    lastTime = millis();
  }
}