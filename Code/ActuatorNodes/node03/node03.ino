// LIBRARIES
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

// ADDRESSES
const uint16_t thisNode = 00; // Address of our actuator node in Octal format
const uint16_t otherNode = 02; // Address of base node

// OBJECTS
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
RF24Network network(radio); //create object to use nRF24L01 in mesh network

// PIN CONNECTIONS
const int relayPin = 4; //relay on digital pin 4
const int ledPin = 8; //LED on digital pin 8
const int acsPin = A0; //ACS712 on analog pin 0

// INITIALIZE VARIABLES
int relayStatus = 0; //store value of relay here
int acsValue = 0; //store value of current sensor here
bool justTurnedOn = true;
int acsLimit = 10; //used for determining whether the device is on or off.

void setup(void)
{
  Serial.begin(115200);
  Serial.println("RF24Network/examples/helloworld_rx/");
  
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication and control
  network.begin(90, thisNode); //(channel, node address)
  
  pinMode(relayPin, OUTPUT); // set relay as output 
  pinMode(ledPin, OUTPUT); // set LED as output
  pinMode(acsPin, INPUT); // set current sensor as input
}

void loop(void){
  
  if(justTurnedOn) //turn on the led for 1 second upon starting
  {
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
    justTurnedOn = false;
  }
  acsValue = analogRead(acsPin);
  Serial.println(acsValue);
  network.update();                  // Check the network regularly
  //=====RECEVING=====//
  while (network.available()) {     // Is there anything ready for us?
    RF24NetworkHeader header;        // If so, grab it and print it out
    int payload = 0;
    network.read(header,&payload,sizeof(payload));
    Serial.println(payload);
    if(payload == 1) // actuator node is triggered using remote node
    {
      // trigger the relay once a payload is received.
      relayStatus = digitalRead(relayPin);
      if(relayStatus == HIGH)
      {
        digitalWrite(relayPin, LOW);
        Serial.println("relaystat is high");
      }
      else
      {
        digitalWrite(relayPin, HIGH);
        Serial.println("relaystat is low");
      }
      
    }

    if (payload == 2) // actuator node is triggered using sensor node
    {
      //check if there is current flowing through the current sensor
      acsValue = analogRead(acsPin);
      if(acsValue > acsLimit) // If there is current flowing, the device is turned on. Thus, trigger the relay.
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
      }
      //else do nothing
    }
  }
}

