int lightPin = 13; 

void setup() {
  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, LOW);
  Serial.begin(9600); 
}

void loop() {
  String nameInMorse = "-- .- .. ... .... .-"; // this mose code is for "Maisha" 
  showMorse(nameInMorse);
  delay(3000); 
}

void showMorse(String morse) {
  int morseLength = morse.length();

  for (int i = 0; i < morseLength; i++) {
    char c = morse[i];

    if (c == '-') {
      digitalWrite(lightPin, HIGH);
      delay(2500);
      digitalWrite(lightPin, LOW);
    } else if (c == '.') {
      digitalWrite(lightPin, HIGH);
      delay(550);
      digitalWrite(lightPin, LOW);
    } else if (c == ' ') {
      delay(1600);
      continue;
    }

    delay(200);
  }
}