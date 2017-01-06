#define SN "DimmableLED"
#define SV "1.1"

#include <MySensor.h> 
#include <SPI.h>

#define LED_PIN_1 3
#define LED_PIN_2 5
#define LED_PIN_3 6
#define FADE_DELAY 10
#define DIMMER_NODE_1 0
#define DIMMER_NODE_2 1
#define DIMMER_NODE_3 2

MySensor gw;

byte currentLevel[3] = {0,0,0};
MyMessage dimmerMsg0(0, V_DIMMER);
MyMessage lightMsg0(0, V_LIGHT);
MyMessage dimmerMsg1(1, V_DIMMER);
MyMessage lightMsg1(1, V_LIGHT);
MyMessage dimmerMsg2(2, V_DIMMER);
MyMessage lightMsg2(2, V_LIGHT);



void setup()  
{ 
  Serial.println( SN ); 
  gw.begin( incomingMessage );
  
  // Register the LED Dimmable Light with the gateway
  gw.present( DIMMER_NODE_1, S_DIMMER );
  gw.wait( 50 );
  gw.present( DIMMER_NODE_2, S_DIMMER );
  gw.wait( 50 );
  gw.present( DIMMER_NODE_3, S_DIMMER );
  gw.wait( 50 );
  
  gw.sendSketchInfo(SN, SV);
  gw.wait( 50 );
  // Pull the gateway's current dim level - restore light level upon sendor node power-up
  gw.request( DIMMER_NODE_1, V_DIMMER );
  gw.wait( 50 );
  gw.request( DIMMER_NODE_2, V_DIMMER );
  gw.wait( 50 );
  gw.request( DIMMER_NODE_3, V_DIMMER );
}

void loop() 
{
  gw.process();
}

void incomingMessage(const MyMessage &message) {
  if (message.type == V_LIGHT || message.type == V_DIMMER) {
    
    //  Retrieve the power or dim level from the incoming request message
    int requestedLevel = atoi( message.data );
    
    // Adjust incoming level if this is a V_LIGHT variable update [0 == off, 1 == on]
    requestedLevel *= ( message.type == V_LIGHT ? 100 : 1 );
    
    // Clip incoming level to valid range of 0 to 100
    requestedLevel = requestedLevel > 100 ? 100 : requestedLevel;
    requestedLevel = requestedLevel < 0   ? 0   : requestedLevel;
    
    Serial.print( "Changing level to " );
    Serial.print( requestedLevel );
    Serial.print( ", from " ); 
    Serial.println( currentLevel[message.sensor] );

    fadeToLevel( requestedLevel, message.sensor);
    
  // Inform the gateway of the current DimmableLED's SwitchPower1 and LoadLevelStatus value...
  // hek comment: Is this really nessesary?
switch(message.sensor)
  {
    case 0:
      gw.send(lightMsg0.set(currentLevel[0] > 0 ? 1 : 0));
      gw.send( dimmerMsg0.set(currentLevel[0]) );
      break;
    case 1:
      gw.send(lightMsg1.set(currentLevel[1] > 0 ? 1 : 0));
      gw.send( dimmerMsg1.set(currentLevel[1]) );
      break;
    case 2:
      gw.send(lightMsg2.set(currentLevel[2] > 0 ? 1 : 0));
      gw.send( dimmerMsg2.set(currentLevel[2]) );
      break;  
  }
    
    }
}

void fadeToLevel( int toLevel, byte sensorId ) {

  int delta = ( toLevel - currentLevel[sensorId] ) < 0 ? -1 : 1;
  
  while ( currentLevel[sensorId] != toLevel ) {
    currentLevel[sensorId] += delta;
  
 switch(sensorId)
  {
    case 0:
      analogWrite( LED_PIN_1, (int)(currentLevel[sensorId] / 100. * 255) );
      break;
    case 1:
      analogWrite( LED_PIN_2, (int)(currentLevel[sensorId] / 100. * 255) );  
      break;
    case 2:
      analogWrite( LED_PIN_3, (int)(currentLevel[sensorId] / 100. * 255) );
      break;      
  }
    
    delay( FADE_DELAY );
  }
}

