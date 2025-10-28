// Arduino Nano 33 IoT Code
// Sends distance readings and listens for commands from Raspberry Pi

#define TRIG_PIN 2
#define ECHO_PIN 3
#define BUZZER_PIN 9

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  float duration = pulseIn(ECHO_PIN, HIGH);
  float distance = (duration * 0.0343) / 2; // cm
  return distance;
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'B') { // Buzz alert
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);
    }
  }

  float dist = measureDistance();
  Serial.print("D:");
  Serial.println(dist);
  delay(200);
}
