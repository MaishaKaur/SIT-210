// --- Smart Glasses Arduino Code ---
// Function: Detect objects using Ultrasonic Sensor and alert using Buzzer
// Also communicates distance data to Raspberry Pi when connected via USB

// Define pin connections
#define TRIG_PIN 2      // Trigger pin of the ultrasonic sensor
#define ECHO_PIN 3      // Echo pin of the ultrasonic sensor
#define BUZZER_PIN 4    // Buzzer output pin

// Variables for sensor data
long duration;          // Stores the time for the sound wave to return
int distance;           // Calculated distance in centimeters
bool piConnected = false; // Tracks whether Raspberry Pi is connected via USB

void setup() {
  Serial.begin(9600);          // Initialize serial communication with Pi (or PC)
  pinMode(TRIG_PIN, OUTPUT);   // Set trigger pin as output
  pinMode(ECHO_PIN, INPUT);    // Set echo pin as input
  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output
}

void loop() {
  // --- Step 1: Send Ultrasonic Pulse ---
  digitalWrite(TRIG_PIN, LOW);   // Make sure trigger starts LOW
  delayMicroseconds(2);          // Short delay to stabilize signal
  digitalWrite(TRIG_PIN, HIGH);  // Send a HIGH pulse for 10 microseconds
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);   // End the pulse

  // --- Step 2: Measure Echo Return ---
  duration = pulseIn(ECHO_PIN, HIGH);   // Measure how long the echo stays HIGH
  distance = duration * 0.034 / 2;      // Convert time into distance (speed of sound = 0.034 cm/µs)

  // --- Step 3: Detect Raspberry Pi Connection ---
  piConnected = Serial;  // If a serial connection is active, assume Pi is connected

  // --- Step 4: Behavior when Pi is Connected ---
  if (piConnected) {
    Serial.println(distance);           // Send distance data to Pi
    if (distance > 0 && distance < 30)  // If object is closer than 30 cm
      digitalWrite(BUZZER_PIN, HIGH);   // Turn buzzer ON
    else
      digitalWrite(BUZZER_PIN, LOW);    // Otherwise turn buzzer OFF
  } 
  // --- Step 5: Behavior when Running Standalone (no Pi) ---
  else {
    if (distance > 0 && distance < 30) {  // If object is near
      digitalWrite(BUZZER_PIN, HIGH);     // Beep shortly
      delay(100);
    } else {
      digitalWrite(BUZZER_PIN, LOW);      // Stay silent
    }
  }

  // --- Step 6: Repeat after a short delay ---
  delay(100);
}
