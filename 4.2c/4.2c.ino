#include <Arduino_LSM6DS3.h>

const int BUTTON_PIN = 2;   
const int LED1_PIN   = 8;   
const int LED2_PIN   = 9;   

volatile bool btnEvent = false;  
bool led1State = LOW;
bool led2State = LOW;

const unsigned long IMU_SAMPLE_MS = 20;   
unsigned long lastImuSample = 0;

float baselineMag = 1.0;
const float alpha = 0.02;       
const float triggerDeltaG = 0.25; 
unsigned long lastMotionToggleMs = 0;
const unsigned long motionCooldownMs = 300;  

void buttonISR() {
  btnEvent = true;  
}

void setup() {
  Serial.begin(115200);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

  // IMU init
  if (!IMU.begin()) {
    Serial.println("IMU init failed! Check board.");
    while (1);
  }
  float x, y, z; 
  for (int i = 0; i < 50; i++) {
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x, y, z);
      float mag = sqrt(x*x + y*y + z*z); // g-units
      baselineMag = (i == 0) ? mag : (0.9f * baselineMag + 0.1f * mag);
      delay(5);
    }
  }

  Serial.println("Ready: Button→LED1, Motion→LED2");
  Serial.println("Shake / jerk the board to toggle LED2.");
}

void loop() {
  if (btnEvent) {
    noInterrupts();
    btnEvent = false;
    interrupts();

    led1State = !led1State;
    digitalWrite(LED1_PIN, led1State);
    Serial.println("Button pressed → LED1 toggled");
  }

  unsigned long now = millis();
  if (now - lastImuSample >= IMU_SAMPLE_MS) {
    lastImuSample = now;

    float x, y, z;
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x, y, z);

      float mag = sqrt(x*x + y*y + z*z);

      baselineMag = (1.0f - alpha) * baselineMag + alpha * mag;

      float deltaG = fabs(mag - baselineMag);

      if (deltaG > triggerDeltaG && (now - lastMotionToggleMs) > motionCooldownMs) {
        lastMotionToggleMs = now;
        led2State = !led2State;
        digitalWrite(LED2_PIN, led2State);
        Serial.print("Motion detected (Δg=");
        Serial.print(deltaG, 3);
        Serial.println(") → LED2 toggled");
      }
    }
  }


}