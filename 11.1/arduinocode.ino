// --- Smart Glasses Arduino Code ---
// Purpose:
//   1. Continuously measure distance using an ultrasonic sensor (HC-SR04).
//   2. Work in two modes automatically:
//      → Connected to Raspberry Pi → sends distance via Serial.
//      → Standalone (only power) → beeps locally when object is too close.

#define TRIG_PIN 2      // Trigger pin of the ultrasonic sensor
#define ECHO_PIN 3      // Echo pin of the ultrasonic sensor
#define BUZZER_PIN 4    // Output pin for active buzzer

long duration;          // Stores pulse duration (in microseconds)
int distance;           // Stores calculated distance (in cm)
bool piConnected = false; // Flag to check if Raspberry Pi is communicating

void setup() {
  Serial.begin(9600);             // Initialize serial communication (for Pi)
  pinMode(TRIG_PIN, OUTPUT);      // Set trigger pin as output
  pinMode(ECHO_PIN, INPUT);       // Set echo pin as input
  pinMode(BUZZER_PIN, OUTPUT);    // Set buzzer pin as output
}

void loop() {
  // --- Step 1: Trigger the ultrasonic pulse ---
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // --- Step 2: Measure the echo response time ---
  // pulseIn() measures the time (in microseconds) for which the Echo pin stays HIGH.
  // Timeout added (25000 µs ≈ 25 ms) to avoid hanging if no echo is received.
  duration = pulseIn(ECHO_PIN, HIGH, 25000);

  // --- Step 3: Convert time into distance (cm) ---
  // Sound travels 0.034 cm per microsecond, divide by 2 for the round trip.
  distance = duration * 0.034 / 2;

  // --- Step 4: Detect if Raspberry Pi is connected ---
  // Serial becomes 'true' when an active USB serial connection exists.
  piConnected = Serial;

  // --- Step 5: Dual operation mode logic ---
  if (piConnected) {
    // -------------------------------
    // MODE 1: Connected to Raspberry Pi
    // -------------------------------
    
    // Send the distance reading to the Pi
    Serial.println(distance);

    // Optional: also beep if the object is very close
    if (distance > 0 && distance < 25)
      digitalWrite(BUZZER_PIN, HIGH);   // Turn on buzzer
    else
      digitalWrite(BUZZER_PIN, LOW);    // Turn off buzzer

  } else {
    // -------------------------------
    // MODE 2: Standalone Operation
    // -------------------------------
    // When not connected to Pi (powered by power bank), 
    // use buzzer alone to alert nearby obstacles.

    if (distance > 0 && distance < 25) {
      digitalWrite(BUZZER_PIN, HIGH);   // Turn on buzzer briefly
      delay(100);                       // Small beep pulse
    } else {
      digitalWrite(BUZZER_PIN, LOW);    // No obstacle → silence
    }
  }

  // --- Step 6: Small delay before next reading ---
  delay(100); // 10 readings per second approx.
}
