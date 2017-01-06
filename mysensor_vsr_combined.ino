#include <MySigningNone.h>
#include <MyTransportNRF24.h>
#include <MyTransportRFM69.h>
#include <MyHwATMega328.h>
#include <MySensor.h>
#include <SPI.h>
#include <DHT.h> 
#include <math.h>
#include <Wire.h>

#define RELAY_1  4  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 4 // Total number of attached relays
#define RELAY_ON 1  // GPIO value to write to turn on attached relay
#define RELAY_OFF 0 // GPIO value to write to turn off attached relay

#define MY_LEDS_BLINKING_FEATURE
#define CHILD_ID_HUM 5
#define CHILD_ID_TEMP 6
#define CHILD_ID_MQ 7
#define HUMIDITY_SENSOR_DIGITAL_PIN 2
const int MQ_Pin = A4;

DHT dht;
float lastTemp;
float lastHum;
float hum_floa;
float last_mq_reading;


MyMessage msgHum(CHILD_ID_HUM, V_HUM);
MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);
MyMessage msgMQ(CHILD_ID_MQ, V_LEVEL);

MyTransportNRF24 radio(RF24_CE_PIN, RF24_CS_PIN, RF24_PA_LEVEL_GW);  
MyHwATMega328 hw;
MySensor gw(radio, hw);


void setup()  
{   
  gw.begin(incomingMessage, 55, false);
  gw.sendSketchInfo("VSR", "1.0");

  dht.setup(HUMIDITY_SENSOR_DIGITAL_PIN); 
  gw.present(CHILD_ID_HUM, S_HUM);
  gw.present(CHILD_ID_TEMP, S_TEMP);
  gw.present(CHILD_ID_MQ, S_AIR_QUALITY);
  
  // Fetch relay status
  for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) 
  {
  // Register all sensors to gw (they will be created as child devices)
  gw.present(sensor, S_LIGHT);
  // Then set relay pins in output mode
  pinMode(pin, OUTPUT);   
  // Set relay to last known state (using eeprom storage) 
  digitalWrite(pin, gw.loadState(sensor)?RELAY_ON:RELAY_OFF);
  }
}


void loop() 
{
  // Alway process incoming messages whenever possible
  gw.process();
  
  delay(dht.getMinimumSamplingPeriod());
  float temperature = dht.getTemperature();
  if (isnan(temperature)) 
  {
      Serial.println("Failed reading temperature from DHT");
  } 
  else if (temperature != lastTemp) 
  {
    lastTemp = temperature;
    gw.send(msgTemp.set(temperature, 1));
    Serial.print("T: ");
    Serial.println(temperature);
  }
  float humidity = dht.getHumidity();
  if (isnan(humidity)) 
  {
      Serial.println("Failed reading humidity from DHT");
  } 
  else if (humidity != lastHum) 
  {
      lastHum = humidity;
      gw.send(msgHum.set(humidity, 1));
      Serial.print("H: ");
      Serial.println(humidity);
  }

float mq_reading = analogRead(MQ_Pin);
if (isnan(mq_reading))
{
      Serial.println("Failed mq_reading");
} 
  else if (mq_reading != last_mq_reading) 
{
    last_mq_reading = mq_reading;
    gw.send(msgMQ.set(mq_reading, 1));
    Serial.print("MQ: ");
    Serial.println(mq_reading);
}


void incomingMessage(const MyMessage &message) 
{
  // We only expect one type of message from controller. But we better check anyway.
  if (message.type==V_LIGHT) 
  {
     // Change relay state
     digitalWrite(message.sensor-1+RELAY_1, message.getBool()?RELAY_ON:RELAY_OFF);
     // Store state in eeprom
     gw.saveState(message.sensor, message.getBool());
     // Write some debug info
     Serial.print("Incoming change for sensor:");
     Serial.print(message.sensor);
     Serial.print(", New status: ");
     Serial.println(message.getBool());
   } 
}
}

