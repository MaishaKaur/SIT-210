#include <SPI.h>              // Enables SPI communication for Wi-Fi module
#include <WiFiNINA.h>         // Enables Wi-Fi functions (connect, send data)
#include <DHT.h>              // Enables temperature and humidity sensor support

#define DHTPIN 2              // DHT sensor data pin connected to D2
#define DHTTYPE DHT22         // Using DHT22 sensor model

char ssid[] = "1313";         // Wi-Fi network name
char pass[] = "1234567a";     // Wi-Fi password

String apiKey = "DCDDKSPB88JM2K3V";          // ThingSpeak channel write key
const char* server = "api.thingspeak.com";   // ThingSpeak server address

WiFiClient client;            // Wi-Fi client for HTTP requests
DHT dht(DHTPIN, DHTTYPE);     // DHT sensor object

unsigned long lastTime = 0;   // Stores last data send time
unsigned long interval = 16000; // Send every 16 seconds

void setup() {
  Serial.begin(9600);         // Start serial monitor
  dht.begin();                // Start DHT sensor
  
  Serial.print("Connecting to WiFi");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) { // Connect to Wi-Fi
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nWiFi connected!");
}

void loop() {
  if (millis() - lastTime > interval) {       // Run every 16 seconds
    float temperature = dht.readTemperature(); // Read temperature (°C)

    String postData = "api_key=" + apiKey + "&field1=" + String(temperature); // Format data

    if (client.connect(server, 80)) {         // Connect to ThingSpeak server
      client.println("POST /update HTTP/1.1");  // HTTP POST request
      client.println("Host: api.thingspeak.com");
      client.println("Connection: close");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(postData.length());
      client.println();
      client.println(postData);               // Send data payload

      Serial.print("Temperature: ");          // Print confirmation
      Serial.print(temperature);
      Serial.println(" °C sent to ThingSpeak!");
    }

    client.stop();                            // Close connection
    lastTime = millis();                      // Update timer
  }
}
