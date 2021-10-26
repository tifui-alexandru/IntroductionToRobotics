const int numberOfLeds = 3;
const int ledPins[] = {9, 10, 11};
const int potPins[] = {A0, A1, A2};

const int minAnalogValue = 0;
const int maxAnalogValue = 1023;

const int minBrightness = 0;
const int maxBrightness = 255;

void setup() {
  for (int i = 0; i < numberOfLeds; ++i) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(potPins[i], INPUT);
  }
  
  Serial.begin(9600);
}

void loop() {
  for (int i = 0; i < numberOfLeds; ++i) {
    int potValue = analogRead(potPins[i]);
    int brightness = map(potValue, minAnalogValue, maxAnalogValue, minBrightness, maxBrightness);
    analogWrite(ledPins[i], brightness);
  }
}