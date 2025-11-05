#include <WiFiNINA.h>        // Include WiFi library for Nano 33 IoT: provides WiFi functions (connect, status, etc.)
#include <PubSubClient.h>    // Include MQTT library: provides publish/subscribe, connect, subscribe, callback

// WiFi credentials
const char* ssid = "IBN-B";        // SSID (network name) used to connect the Arduino to your Wi-Fi
const char* password = "CUPunjab"; // Wi-Fi password for the SSID above

// MQTT broker settings - using public Mosquitto test broker
const char* mqttServer = "test.mosquitto.org";  // Domain or IP of the MQTT broker (where messages are sent/received)
const int mqttPort = 1883;                      // Network port for MQTT (1883 = standard unsecured MQTT)
const char* topic = "SIT210/wave";              // Topic string used as the channel for publish/subscribe

// Pin definitions
const int trigPin = 6;   // Digital pin connected to HC-SR04 TRIG (used to send the ultrasonic pulse)
const int echoPin = 8;   // Digital pin connected to HC-SR04 ECHO (used to listen for the echo pulse)
const int ledPin = 3;    // Digital pin connected to an LED (used to show received messages / feedback)

// Ultrasonic sensor variables
long duration;                          // Variable to store the echo pulse duration in microseconds
int distance;                           // Variable to store calculated distance in centimeters
const int detectionThreshold = 20;      // Threshold distance (cm) below which we consider a "wave" detected

// MQTT client objects
WiFiClient wifiClient;                  // WiFi client object: handles TCP/IP socket operations over WiFi
PubSubClient client(wifiClient);        // MQTT client object using the WiFi client for network transport

// SETUP 
void setup() {
  Serial.begin(9600);                   // Start serial communication at 9600 bps for debugging/output to PC
  
  pinMode(trigPin, OUTPUT);             // Set trigger pin as OUTPUT so Arduino can send pulses to ultrasonic sensor
  pinMode(echoPin, INPUT);              // Set echo pin as INPUT to receive the echo pulse from the sensor
  pinMode(ledPin, OUTPUT);              // Set LED pin as OUTPUT to control the LED
  digitalWrite(ledPin, LOW);            // Ensure LED is OFF at startup (drive LOW)

  // Connect to WiFi
  WiFi.begin(ssid, password);           // Start connecting to Wi-Fi using ssid and password
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); } // Wait until WiFi connects (blocking loop)

  // Configure MQTT broker and callback
  client.setServer(mqttServer, mqttPort); // Tell MQTT client which broker and port to use for connections
  client.setCallback(callback);           // Register the callback function called when an MQTT message arrives

  // Connect to MQTT broker
  client.connect("ArduinoWaveClient");    // Connect to the MQTT broker with a client ID ("ArduinoWaveClient")
  client.subscribe(topic);                // Subscribe to the specified topic so messages on it are received
}

// MAIN LOOP
void loop() {
  // Ensure MQTT connection stays active
  if (!client.connected()) {              // Check if MQTT client is connected to broker
    client.connect("ArduinoWaveClient");  // If not connected, attempt to connect again
    client.subscribe(topic);              // Re-subscribe to topic after reconnecting
  }
  client.loop();                          // Process incoming MQTT messages, maintain connection keepalive

  // Continuously check if a wave is detected
  checkForWave();                         // Call the function that triggers the ultrasonic sensor and publishes on detection

  delay(100);                             // Small delay to reduce CPU usage and pace loop iterations
}

// WIFI CONNECTION 
void connectToWiFi() {                    // (NOT used in this exact sketch version but often used to modularize WiFi logic)
  Serial.print("Connecting to WiFi");    // Print status that WiFi connection attempt is starting
  WiFi.begin(ssid, password);             // Start WiFi connection attempt
  while (WiFi.status() != WL_CONNECTED) { // Wait in a loop until the WiFi becomes connected
    delay(500);                           // Wait 500 ms between checks to avoid busy looping
    Serial.print(".");                    // Print a dot for each retry (visual feedback)
  }
  Serial.println("\nConnected to WiFi!"); // Print success message when connected
}

//  MQTT CONNECTION 
void connectToMQTT() {                    // (NOT used in the simplified flow above; usually modularizes MQTT connect/retry)
  while (!client.connected()) {           // Loop until the client is connected to the broker
    Serial.print("Connecting to MQTT..."); // Debugging output showing we are trying to connect
    if (client.connect("ArduinoWaveClient")) { // Try to connect with a client ID
      Serial.println("connected!");        // Print confirmation if connected
      client.subscribe(topic);             // Subscribe to topic after successful connection
    } else {
      Serial.print("failed, rc=");         // Print result code when connection failed (debug)
      Serial.print(client.state());        // Print MQTT client state code to diagnose issue
      Serial.println(" retrying in 5 seconds"); // Inform user about retry delay
      delay(5000);                         // Wait before retrying to avoid tight loop on failure
    }
  }
}

// CALLBACK FUNCTION
// Handles incoming MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  // Convert the payload (byte array) into a readable string
  String message;                         // Create empty String to build the message text
  for (int i = 0; i < length; i++) {      // Loop over incoming payload bytes
    message += (char)payload[i];          // Append each byte cast to char to form the message string
  }

  // Debug: print received message
  Serial.print("Message arrived on topic: "); // Print the topic where message was received
  Serial.println(topic);                   // Print topic string
  Serial.print("Message: ");               // Print label for message content
  Serial.println(message);                 // Print actual message payload as text

  // If the message contains the word "pat", flash LED quickly 5 times
  if (message.indexOf("pat") >= 0) {       // Check if "pat" substring exists in the message (-1 if not found)
    Serial.println("Pat command received! Flashing LED 5 times quickly."); // Debug print for this branch
    for (int i = 0; i < 5; i++) {          // Loop 5 times to blink LED as a special response
      digitalWrite(ledPin, HIGH);          // Turn LED ON
      delay(500);                          // Keep LED on for 500 ms (visible blink)
      digitalWrite(ledPin, LOW);           // Turn LED OFF
      delay(500);                          // Pause between blinks for 500 ms
    }
  } else {
    // Otherwise, flash LED 3 times at a slower pace
    Serial.println("Standard message received. Flashing LED 3 times."); // Debug print for default branch
    for (int i = 0; i < 3; i++) {          // Loop 3 times to blink LED as standard response
      digitalWrite(ledPin, HIGH);          // Turn LED ON
      delay(200);                          // Keep LED on for 200 ms (short blink)
      digitalWrite(ledPin, LOW);           // Turn LED OFF
      delay(200);                          // Pause between blinks for 200 ms
    }
  }
}

// WAVE DETECTION 
void checkForWave() {
  // Clear the trigger pin
  digitalWrite(trigPin, LOW);             // Ensure trigger pin starts LOW to prepare for a clean pulse
  delayMicroseconds(2);                   // Short pause (2 microseconds) to stabilize the pin state

  // Send a 10us HIGH pulse to trigger ultrasonic sensor
  digitalWrite(trigPin, HIGH);            // Set trigger HIGH to start ultrasonic burst
  delayMicroseconds(10);                  // Maintain HIGH for 10 microseconds as required by sensor
  digitalWrite(trigPin, LOW);             // Set trigger LOW to finish the pulse

  // Read echo pin, duration of sound wave round trip (in microseconds)
  duration = pulseIn(echoPin, HIGH);      // Measure the length of the incoming HIGH pulse on echo pin (microseconds)

  // Calculate distance (cm) using speed of sound (0.034 cm/us), divided by 2 for round trip
  distance = duration * 0.034 / 2;        // Convert time to distance (cm): time × speed_of_sound / 2

  // If object is within detection threshold -> register a "wave"
  if (distance > 0 && distance < detectionThreshold) { // Check valid positive distance and within threshold
    Serial.println("Wave detected!");     // Print a short debug message indicating detection
    Serial.print("Distance: ");           // Print label for measured distance
    Serial.println(distance);             // Print measured distance value in cm

    // Publish a message to the MQTT broker
    String message = "Wave from Maisha";  // Create the message payload to send
    client.publish(topic, message.c_str()); // Publish payload to the configured MQTT topic (c_str() gives const char*)

    // Flash LED once as confirmation
    digitalWrite(ledPin, HIGH);           // Turn LED on briefly to confirm publishing action
    delay(500);                           // Keep LED on for 500 ms for visible confirmation
    digitalWrite(ledPin, LOW);            // Turn LED off

    // Delay to prevent multiple detections from one wave
    delay(1000);                          // Wait 1 second to avoid repeated messages from same gesture
  }
}
