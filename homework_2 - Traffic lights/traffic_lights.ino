// led constants & variables
const int red = 0;
const int green = 1;
const int yellow = 2;

const int carsLedPins[] = {8, 10, 9};
const int peopleLedPins[] = {12, 13, 9};

const unsigned long blinkingPeriod = 250;
unsigned long numberOfBlinks = 0;

bool blinkingLedState = LOW;

// button constants & variables
const int buttonPin = 2;

bool buttonState = HIGH;
bool prevState = HIGH;

int reading = HIGH;
int prevReading = HIGH;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// buzzer constants & variables
const int buzzerPin = 11;
const int buzzerTone = 700;

const unsigned long state3BuzzPeriod = 500;

unsigned long numberOfBuzzSounds = 0;
bool buzzerState = LOW;

// general constants & variables
const unsigned long buttonCountdown = 10000;

const unsigned long state2Duration = 3000;
const unsigned long state3Duration = 10000;
const unsigned long state4Duration = 5000;

unsigned long state2StartTime = 0;
unsigned long state3StartTime = 0;
unsigned long state4StartTime = 0;

const unsigned long countdownDuration = 10000;
unsigned long countdownStart = 0;

enum stateType {
  stateType1,
  stateType2,
  stateType3,
  stateType4,
  stateType2Countdown
};

stateType currentState = stateType1;

/*
 * State1:
 *    Green for cars
 *    Red for people
 *    No sounds
 */
void state1() {
  // begin state 1
  digitalWrite(carsLedPins[green], HIGH);
  digitalWrite(peopleLedPins[red], HIGH);

  // detect button press
  reading = digitalRead(buttonPin);

  if (reading != prevReading) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        countdownStart = millis();
        currentState = stateType2Countdown;
      }
    }
  }

  prevReading = reading;
}

/*
 * State2:
 *    Yellow for cars
 *    Red for people
 *    No sounds
 */
void state2CountDown() {
  if (millis() - countdownStart > countdownDuration) {
    // restore and go to state 2
    digitalWrite(carsLedPins[green], LOW);
    state2StartTime = millis();
    currentState = stateType2;
  }
}
 
void state2() {
  if (millis() - state2StartTime <= state2Duration) {
    // begin state 2
    digitalWrite(carsLedPins[yellow], HIGH);
  }
  else {
    // restore and go to state 3
    digitalWrite(carsLedPins[yellow], LOW);
    digitalWrite(peopleLedPins[red], LOW);
    
    state3StartTime = millis();
    currentState = stateType3;
  }
}

/*
 * State3:
 *    Red for cars
 *    Green for people
 *    Beeping
 */
void state3() {
  if (millis() - state3StartTime <= state3Duration) {
    // begin state 3
    digitalWrite(carsLedPins[red], HIGH);
    digitalWrite(peopleLedPins[green], HIGH); 

    if (millis() - state3StartTime - numberOfBuzzSounds * state3BuzzPeriod > state3BuzzPeriod) {
      if (buzzerState == LOW) {
        tone(buzzerPin, buzzerTone);
      }
      else {
        noTone(buzzerPin);
      }

      numberOfBuzzSounds++;
      buzzerState = !buzzerState;
    }
  }
  else {
    // restore and go to state 4
    numberOfBuzzSounds = 0;
    buzzerState = LOW;
    state4StartTime = millis();
    currentState = stateType4;
  }
}

/*
 * State4:
 *    Red for cars
 *    Blinking Green for people
 *    Beeping faster
 */
void state4() {
  if (millis() - state4StartTime <= state4Duration) {
    // begin state 4 by making pepole green led blink
    // buzzer and led "blink at the same time"
    if (millis() - state4StartTime - numberOfBlinks * blinkingPeriod > blinkingPeriod) {
      if (blinkingLedState == LOW) {
        digitalWrite(peopleLedPins[green], HIGH);
      }
      else {
        digitalWrite(peopleLedPins[green], LOW);
      }

      if (buzzerState == LOW) {
        tone(buzzerPin, buzzerTone);
      }
      else {
        noTone(buzzerPin);
      }
      
      numberOfBlinks++;
      blinkingLedState = !blinkingLedState;
      buzzerState = !buzzerState;
    }
  }
  else {
    // restore and go to state 1
    digitalWrite(carsLedPins[red], LOW);
    digitalWrite(peopleLedPins[green], LOW);
    
    numberOfBlinks = 0;
    blinkingLedState = HIGH;
    noTone(buzzerPin);

    currentState = stateType1;
  }
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  
  for (int i = 0; i < 3; ++i) {
     pinMode(carsLedPins[i], OUTPUT);
  }

  for (int i = 0; i < 3; ++i) {
    pinMode(peopleLedPins[i], OUTPUT);
  }
}

void loop() {  
  if (currentState == stateType1) {
    state1();
  }
  else if (currentState == stateType2) {
    state2();
  }
  else if (currentState == stateType3) {
    state3();
  }
  else  if (currentState == stateType4) {
    state4();
  }
  else {
    state2CountDown();
  }
}
