enum { nkinds=2, debounced=5 }; // # of activity kinds;  debounce-criterion
const byte outPins[nkinds] = {11}; // Output pin numbers
const byte inPins[nkinds]  = {2, 3}; // Input pin numbers
const unsigned int actLens[nkinds] = {1800000, 3600000}; // Activity lengths, ms
byte states[nkinds] = {0, 0};
unsigned long actEnds[nkinds] = {0, 0};
int lightState = 0;
const int lightPin = 4;
const int lightrelePin =  13;


void setup() {
// Set input/output modes of pins
  for (byte i=0; i<nkinds; ++i) {
    pinMode(outPins[i], OUTPUT);
    pinMode(inPins[i], INPUT);
  }
  pinMode(lightrelePin, OUTPUT);
  pinMode(lightPin, INPUT);
}

// Once per millisecond, for each kind of activity
// check for new beginning or deal with ongoing action
void loop() {
  lightState = digitalRead(lightPin);
  if (lightState == HIGH) {
    digitalWrite(lightrelePin, HIGH);  
  } 
  else {
    // turn LED off:
    digitalWrite(lightrelePin, LOW); 
  }
  
  unsigned long now = millis();
  for (byte i=0; i<nkinds; ++i) {
    if (states[i] < debounced) {  // Is activity off?
      // It's off, look for a turnon
      if (digitalRead(inPins[i])) {
        ++states[i];        // Button is closed at the moment
        // See if button has been debounced
        if (states[i] >= debounced) { // If so, start its action
          digitalWrite(outPins[i], HIGH);
          actEnds[i] = now + actLens[i];
        }
      }
      else {
        states[i] = 0;      // Button is not closed at moment
      }
    }
    else { // It's on, see if end-time has arrived
      if (now >= actEnds[i]) {
        digitalWrite(outPins[i], LOW); // Clear output when done
        states[i] = 0;        // Return to button-seeking state
      }
    }
  }
  while (now == millis()) {};       // Finish current millisecond
}

