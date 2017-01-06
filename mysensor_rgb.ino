// MySensors
#include <MySigningNone.h>
#include <MyTransportNRF24.h>
#include <MySensor.h>
#include <SPI.h>
#include <Wire.h>

MySensor gw;

#define RED_PIN 3
#define GREEN_PIN 5
#define BLUE_PIN 6

#define SAVE_LIGHT_STATE 1
#define SAVE_DIMMER_LEVEL 2
#define SAVE_LIGHT_COLOR 3

#define RELAY_ON 1
#define RELAY_OFF 0

long RGB_values[3] = {0,0,0};
String hexstring;

int RValue;
int GValue;
int BValue;
int CurrentLevel;

void setup() 
{
  gw.begin(incomingMessage, 36);
  gw.sendSketchInfo("RGB LED Dimmer", "1.0");
  gw.present(0, S_RGB_LIGHT);
  Serial.print("SYSTEM STARTED");
  // Set pin output mode
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
}

void loop() 
{
gw.process();
}
void incomingMessage(const MyMessage &message) {
  if (message.type==V_RGB) {
    hexstring = message.getString();
    Serial.print("RGB command: ");
    Serial.println(hexstring);
    setColor(hexstring);
  }
  else if (message.type==V_DIMMER) {
    int dimLevel = message.getInt(); //0-100%
    Serial.print("Dim command: ");
    Serial.println(dimLevel);
    setDimLevel(dimLevel);
    gw.saveState(SAVE_DIMMER_LEVEL, dimLevel);
  }
  else if (message.type==V_LIGHT) {
   if(message.getBool() == RELAY_ON) {
    setColor("FFFFFF");
    gw.saveState(SAVE_LIGHT_STATE, RELAY_ON);
   }
   if(message.getBool() == RELAY_OFF) {
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
    gw.saveState(SAVE_LIGHT_STATE, RELAY_OFF);
   }
  }
}

void setDimLevel(int level) {
  level = level > 100 ? 100 : level;
  level = level < 0 ? 0: level;
  int delta = ( level - CurrentLevel) < 0 ? -1 : 1;
  RValue = (int)(level / 100. * RValue);
  BValue = (int)(level / 100. * BValue);
  GValue = (int)(level / 100. * GValue);
  
  analogWrite(RED_PIN, RValue);
  analogWrite(GREEN_PIN, GValue);
  analogWrite(BLUE_PIN, BValue);
  
  CurrentLevel = level;
}

void setColor(String hexstring) {
  long number = (long) strtol( &hexstring[0], NULL, 16);
  Serial.print("Color long: ");
  Serial.println(number);
  RValue = number >> 16;
  GValue = number >> 8 & 0xFF;
  BValue = number & 0xFF;

  Serial.print("Color: ");
  Serial.println(hexstring);
  Serial.print("Red: ");
  Serial.println(RValue);
  Serial.print("Green: ");
  Serial.println(GValue);
  Serial.print("Blue: ");
  Serial.println(BValue);
  
  analogWrite(RED_PIN, RValue);
  analogWrite(GREEN_PIN, GValue);
  analogWrite(BLUE_PIN, BValue);
}

