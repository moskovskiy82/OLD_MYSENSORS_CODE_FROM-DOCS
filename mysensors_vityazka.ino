#include <SPI.h>
#include <MySensor.h>  
#include <DHT.h> 
#include <math.h>
#include <Wire.h>
#define MY_LEDS_BLINKING_FEATURE
#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_MQ 3
#define HUMIDITY_SENSOR_DIGITAL_PIN 8
const int MQ_Pin = A3;

MySensor gw;
DHT dht;

//VARIABLES FOR MY SENSORS
float lastTemp;
float lastHum;
float hum_floa;
float last_mq_reading;
//boolean metric = true; 
MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgMQ(CHILD_ID_MQ, V_LEVEL);

//SETTING PINS FOR BUTTONS RELE
int led_pin = 3; //STATE LED
int rele_svet = 4;       // the number of the output pin
int rele_vent = 8;       // the number of the output pin

int button_light = 7;         // the number of the input pin
int button_30m = 5;
int button_60m = 6;         // the number of the input pin

//Переменные для всякой хуйни
int light_state = HIGH;      // the current state of the output pin
int light_reading;           // the current reading from the input pin
int light_previous;    // the previous reading from the input pin
int rele_state = HIGH;      // the current state of the output pin
int rele1_state = HIGH;      // the current state of the output pin
int run30;
int run60;
int button_30m_state = HIGH;
int button_30m_reading;
int button_30m_previous;
int button_60m_state = HIGH;
int button_60m_reading;
int button_60m_previous;

//Функция millis
long time = 0;         // the last time the output pin was toggout
long debounce = 50;   // the debounce time, increase if the output flickers
unsigned long previousMillis = 0;
long tm_30m = 300000; //300 000 = 5 minutes
long tm_60m = 1800000; // 1800 000 = 30 minutes

void setup()
{
  Serial.begin(9600);
  gw.begin(NULL,72,false);
  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN);
  gw.sendSketchInfo("Humidity", "1.0");
  gw.sendSketchInfo("Air quality", "1.0");
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_MQ, S_AIR_QUALITY);
  pinMode(button_light, INPUT_PULLUP);
  pinMode(button_30m, INPUT_PULLUP);
  pinMode(button_60m, INPUT_PULLUP);
  //OUTPUT
  pinMode(rele_svet, OUTPUT);
  pinMode(rele_vent, OUTPUT);
  pinMode(led_pin, OUTPUT);

  digitalWrite(rele_vent, HIGH);
  digitalWrite(rele_svet, HIGH);
  
}

void loop(){

// ######### SETTING MILLIS AND CROSS FUNCTION
  unsigned long currentMillis = millis();
  light_reading = digitalRead(button_light);
  button_30m_reading = digitalRead(button_30m);
  button_60m_reading = digitalRead(button_60m);

  if (light_reading == HIGH && light_previous == LOW && millis() - time > debounce) {
    if (light_state == LOW) {
      light_state = HIGH;
      digitalWrite(rele_svet, light_state);
      Serial.println("LIGHT OFF");
    }
    else {
      light_state = LOW;
      digitalWrite(rele_svet, light_state);
      Serial.println("LIGHT ON");
      time = millis();
    }
  }


  //PROVERAEM KAK DOLGO RABOTAET VITYAZKA
  if  (run30 && rele_state == LOW && (currentMillis - previousMillis >= tm_30m))
  {
    rele_state = HIGH;
    digitalWrite(rele_vent, rele_state);
    rele1_state = HIGH;
    previousMillis = currentMillis;
    Serial.println("MILLIS RELE OFF");
  }
  if  (run60 && rele1_state == LOW && (currentMillis - previousMillis >= tm_60m))
  {
    rele_state = HIGH;
    rele1_state = HIGH;
    digitalWrite(rele_vent, rele1_state);
    previousMillis = currentMillis;
    Serial.println("MILLIS RELE1 OFF");
  }

  /////////////////////////KNOPKA1 5 minute
  if (button_30m_reading == HIGH && button_30m_previous == LOW && millis() - time > debounce) {
    if (rele_state == LOW)
    {
      rele_state = HIGH;
      digitalWrite(rele_vent, rele_state);
      run30 = false;
      Serial.println("RELE OFF");
    }
    else {
      rele_state = LOW;
      digitalWrite(rele_vent, rele_state);
      run30 = true;
      run60 = false;
      Serial.println("RELE ON");
      previousMillis = millis();
      time = millis();
    }
  }
  /////////////////////////KNOPKA2 30 minute
  if (button_60m_reading == HIGH && button_60m_previous == LOW && millis() - time > debounce) {
    if (rele1_state == LOW)
    { rele1_state = HIGH;
      digitalWrite(rele_vent, rele1_state);
      run60 = false;
      Serial.println("RELE1 OFF");
    }
    else {
      rele1_state = LOW;
      digitalWrite(rele_vent, rele1_state);
      run60 = true;
      run30 = false;
      previousMillis = millis();
      Serial.println("RELE1 ON");
      time = millis();
    }
  }

   delay(dht.getMinimumSamplingPeriod());

  float temperature = dht.getTemperature();
  if (isnan(temperature)) {
      Serial.println("Failed reading temperature from DHT");
  } else if (temperature != lastTemp) {
    lastTemp = temperature;
    gw.send(msgTemp.set(temperature, 1));
    Serial.print("T: ");
    Serial.println(temperature);
  }
  float humidity = dht.getHumidity();
  if (isnan(humidity)) {
      Serial.println("Failed reading humidity from DHT");
  } else if (humidity != lastHum) {
      lastHum = humidity;
      gw.send(msgHum.set(humidity, 1));
      Serial.print("H: ");
      Serial.println(humidity);


float mq_reading = analogRead(MQ_Pin);
  if (isnan(mq_reading)) {
      Serial.println("Failed mq_reading");
  } else if (mq_reading != last_mq_reading) {
    last_mq_reading = mq_reading;
    
    gw.send(msgMQ.set(mq_reading, 1));
    Serial.print("MQ: ");
    Serial.println(mq_reading);
  }
  }
//############ SET SOME SHIT
  light_previous = light_reading;
  button_30m_previous = button_30m_reading;
  button_60m_previous = button_60m_reading;
}
