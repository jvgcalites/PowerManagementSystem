// LIBRARIES
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

// ADDRESSES
const uint16_t thisNode_addr = 01; // Address of our actuator node in Octal format
const uint16_t baseNode_addr = 00; // Address of base node

// NRF24L01
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
RF24Network network(radio); //create object to use nRF24L01 in mesh network

// PIN CONNECTIONS
const int relayPin = 4; //relay on digital pin 4
const int ledPin = 8; //LED on digital pin 8
const int acsPin = A0; //ACS712 on analog pin 0

// ACS712 CURRENT SENSOR
int mVperAmp = 100; // use 185 for 10A Module, 100 for 20A Module, and 66 for 30A Module
double voltage = 0;
double vRMS = 0;
double ampsRMS = 0;
double acsThresholdValue = 0.15; //used for determining whether the device is on or off.

// INITIALIZE VARIABLES
int relayStatus = 0; //store value of relay here
bool justTurnedOn = true;
int payload = 0; 

void setup(void)
{
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication and control
  network.begin(90, thisNode_addr); //(channel, node address)
  
  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT); 
  pinMode(acsPin, INPUT); 
}

void loop(void){

  //turn ON the led for 3 seconds upon starting
  if(justTurnedOn)
  {
    digitalWrite(ledPin, HIGH);
    delay(3000);
    digitalWrite(ledPin, LOW);
    justTurnedOn = false;
  }

  // Check the network regularly
  network.update();                  
  if(network.available()) 
  {
    RF24NetworkHeader header;
    network.read(header,&payload,sizeof(payload));

    // if actuator node is triggered using remote node
    if(payload == 1)
    {
      // trigger the relay once the payload is received.
      relayStatus = digitalRead(relayPin);
      if(relayStatus == HIGH)
      {
        digitalWrite(relayPin, LOW);
      }
      else
      {
        digitalWrite(relayPin, HIGH);
      }

      blinkLed();
    }

    // if actuator node is triggered using sensor node
    if (payload == 2)
    {
      // get AC current reading
      voltage = getVVP();
      vRMS = (voltage/2.0) *0.707;  //root 2 is 0.707
      ampsRMS = (vRMS * 1000)/mVperAmp;

      // if there is current flowing, the device is turned on. Thus, trigger the relay.
      if(ampsRMS > acsThresholdValue)
      {
        relayStatus = digitalRead(relayPin);
        if(relayStatus == HIGH)
        {
          digitalWrite(relayPin, LOW);
        }
        else
        {
          digitalWrite(relayPin, HIGH);
        }
        blinkLed();
      }
    }
  }
}

float getVVP(){
  float result;
  float acsValue;
  int maxValue = 0;
  int minValue = 1024;

  uint32_t start_time = millis();
  while((millis() - start_time) < 1000) // get sample for 1 second
  {
    acsValue = analogRead(acsPin);
    if(acsValue > maxValue){
      maxValue = acsValue;
    }
    if(acsValue < minValue){
      minValue = acsValue;
    }
  }
  result = ((maxValue - minValue) * 5.0) / 1024;
  
  return result;
}

void blinkLed()
{
  //blink led showing data sent status
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
}

