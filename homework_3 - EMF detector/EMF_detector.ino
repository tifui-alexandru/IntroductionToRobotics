// antenna constants & variables
const int antennaPin = A5;
unsigned int antennaValue;
unsigned int averageValue;

// buzzer constants & variables
const int buzzerPin = 11;
const int buzzerTone = 700;

// 7-segment display constants & variables
const int pinA = 2;
const int pinB = 3;
const int pinC = 4;
const int pinD = 5;
const int pinE = 6;
const int pinF = 7;
const int pinG = 8;

const int segSize = 7;
const int noOfDigits = 10;

int segments[segSize] = { pinA, pinB, pinC, pinD, pinE, pinF, pinG };

bool digitMatrix[noOfDigits][segSize] = {
  // a  b  c  d  e  f  g
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 1, 1, 0, 0, 0, 0}, // 1
  {1, 1, 0, 1, 1, 0, 1}, // 2
  {1, 1, 1, 1, 0, 0, 1}, // 3
  {0, 1, 1, 0, 0, 1, 1}, // 4
  {1, 0, 1, 1, 0, 1, 1}, // 5
  {1, 0, 1, 1, 1, 1, 1}, // 6
  {1, 1, 1, 0, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 0, 1, 1}  // 9
};

const unsigned int displayDelay = 40;
unsigned int displayTime = 0;

//constants & variables for interpreting the emf signal

const int minSenseLimit = 0;
const int maxSenseLimit = 600;

const int digitSignalLimit[noOfDigits] = { 100, 150, 200, 250, 300, 350, 400, 450, 500, maxSenseLimit };

const unsigned int noSamples = 500;

const byte minDigitDisplay = 0;
const byte maxDigitDisplay = noOfDigits - 1;

void displayNumber(byte digit) {
  for (int i = 0; i < segSize; ++i) {
    digitalWrite(segments[i], digitMatrix[digit][i]);
  }
}

void setup() {
  for (int i = 0; i < segSize; ++i) {
    pinMode(segments[i], OUTPUT);
  }

  pinMode(antennaPin, INPUT);
}

unsigned int getAverageValue() {
  // get the average of noSamples readings
  
  float answer = 0;
  for (int i = 0; i < noSamples; ++i) {
    antennaValue = 1.0 * analogRead(antennaPin);
    answer += antennaValue;
  }
  
  answer /= noSamples;
  answer = constrain(answer, minSenseLimit, maxSenseLimit);

  // determine to which digit does the signal correspond to
  for (int i = 0; i < noOfDigits; ++i) {
    if (answer < digitSignalLimit[i]) {
      answer = i;
      break;
    }
  }

  return answer;
}

void loop() {
  if (millis() - displayTime > displayDelay) {
    averageValue = getAverageValue();
    displayNumber(averageValue);
    tone(buzzerPin, buzzerTone * averageValue);

    displayTime = millis();
  }
}
