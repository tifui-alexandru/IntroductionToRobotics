#include <EEPROM.h>

// shift-register constants
const int dataPin = 12; // DS
const int latchPin =  11; // STCP
const int clockPin = 10; // SHCP

// 4 digit 7-segment display constants & variables
const int noOfDigits = 10;
int digitArray[noOfDigits] = {
  //A B C D E F G DP
  B11111100, // 0
  B01100000, // 1
  B11011010, // 2
  B11110010, // 3
  B01100110, // 4
  B10110110, // 5
  B10111110, // 6
  B11100000, // 7
  B11111110, // 8
  B11110110, // 9
};

const int segD1 = 7;
const int segD2 = 6;
const int segD3 = 5;
const int segD4 = 4;

const int displayCount = 4;

const int displayDigits[displayCount] = {
  segD1, segD2, segD3, segD4
};

int memorizedNumber = 0;
int activeDigit = 0;

const unsigned long blinkingInterval = 500;
unsigned long blinkingStart = 0;
bool blinkingState = HIGH;

// joystick constants & variables
const int pinSW = 2; // interrupt pin

// swap pinX and pinY for better UX
const int pinX = A1;
const int pinY = A0;

bool joyMoved = false;
int minThreshold = 400;
int maxThreshold = 600;

bool swState;
int xValue = 0;
int yValue = 0;

// state constants & variables
enum stateType {
  stateType1,
  stateType2,
};

volatile stateType currentState = stateType1;
const unsigned long debounceInterval = 200;

// EEPROM constants & variables

const int byteMask = 0xFF;
const int byteSize = 8;

void setup() {
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  pinMode(pinSW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinSW), changeState, FALLING);

  for (int i = 0; i < displayCount; i++) {
    pinMode(displayDigits[i], OUTPUT);
    digitalWrite(displayDigits[i], LOW);
  }
  Serial.begin(9600);
}

void loop() {
  //  Serial.println(currentState);
  if (currentState == stateType1) {
    state1();
  }
  else {
    state2();
  }
}

/*
   State1:
    the memorized number is displayed
    blinking decimal point of the current digit
    move the joystick on the horizontal axis to change the current digit
    push the button to select the digit and enter state2
*/
void state1() {
  memorizedNumber = readFromEEPROM();
  xValue = analogRead(pinX);

  // swap increase and decrease logic for better UX
  if (xValue < minThreshold and joyMoved == false) {
    activeDigit = (activeDigit + 1) % displayCount;
    joyMoved = true;
  }

  if (xValue > maxThreshold and joyMoved == false) {
    activeDigit = (activeDigit - 1 + displayCount) % displayCount;
    joyMoved = true;
  }

  if (xValue >= minThreshold and xValue <= maxThreshold) {
    joyMoved = false;
  }

  unsigned long now = millis();
  if (now - blinkingStart > blinkingInterval) {
    blinkingState = !blinkingState;
    blinkingStart = now;
  }

  writeNumber(memorizedNumber, blinkingState);
}

/*
   State2:
    the memorized number is displayed
    decimal point of the current digit is on
    move the joystick on the vertical axis to change the current digit's value
    push the button to select the digit's value and enter state1 again
*/
void state2() {
  int currentDigitValue = getNthDigitValue(memorizedNumber, activeDigit);

  yValue = analogRead(pinY);

  if (yValue < minThreshold and joyMoved == false) {
    currentDigitValue = (currentDigitValue - 1 + noOfDigits) % noOfDigits;
    memorizedNumber = setNthDigitValue(memorizedNumber, activeDigit, currentDigitValue);
    joyMoved = true;
  }

  if (yValue > maxThreshold and joyMoved == false) {
    currentDigitValue = (currentDigitValue + 1) % noOfDigits;
    memorizedNumber = setNthDigitValue(memorizedNumber, activeDigit, currentDigitValue);
    joyMoved = true;
  }

  if (yValue >= minThreshold and yValue <= maxThreshold) {
    joyMoved = false;
  }

  writeNumber(memorizedNumber, HIGH);
  writeToEEPROM(memorizedNumber);
}

void changeState() {
  static unsigned long lastChangeTime = 0;
  unsigned long now = millis();

  if (now - lastChangeTime > debounceInterval) {
    currentState ^= stateType1 ^ stateType2;
  }

  lastChangeTime = now;
}

int getNthDigitValue(int number, int digitPosition) {
  for (int i = 0; i < digitPosition; ++i) {
    number /= 10;
  }

  return number % 10;
}

int setNthDigitValue(int number, int digitPosition, int newDigitValue) {
  int answer = 0;
  int curr10power = 1;

  for (int i = 0; i < digitPosition; ++i) {
    answer = curr10power * (number % 10) + answer;
    number /= 10;
    curr10power *= 10;
  }

  answer = curr10power * newDigitValue + answer;
  number /= 10;
  curr10power *= 10;

  for (int i = digitPosition + 1; i < displayCount; ++i) {
    answer = curr10power * (number % 10) + answer;
    number /= 10;
    curr10power *= 10;
  }

  return answer;
}

void writeNumber(int number, bool stateOfDP) {
  int currentNumber = number;
  int lastDigit;
  int displayValue;

  // take the last displayCount digits of the number
  for (int displayDigit = 0; displayDigit < displayCount; ++displayDigit)  {
    lastDigit = currentNumber % 10;
    displayValue = digitArray[lastDigit];

    if (displayDigit == activeDigit and stateOfDP == HIGH) {
      // turn on DP
      displayValue |= 1;
    }

    showDigit(displayDigit);
    writeReg(displayValue);

    currentNumber /= 10;
    delay(5);
  }
}

void showDigit(int digit) {
  for (int i = 0; i < displayCount; ++i) {
    digitalWrite(displayDigits[i], HIGH);
  }

  digitalWrite(displayDigits[digit], LOW);
}

void writeReg(int digit) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, digit);
  digitalWrite(latchPin, HIGH);
}

void writeToEEPROM(int number) {
  // max value of number is 9999
  // it is sufficient to only write the 2 least significant bytes of number in EEPROM
  byte numberByte1 = number & byteMask;
  byte numberByte2 = (number >> byteSize) & byteMask;

  EEPROM.update(0, numberByte1);
  EEPROM.update(1, numberByte2);
}

int readFromEEPROM() {
  byte byte1Value = EEPROM.read(0);
  byte byte2Value = EEPROM.read(1);
  return (byte2Value << byteSize) | byte1Value;
}
