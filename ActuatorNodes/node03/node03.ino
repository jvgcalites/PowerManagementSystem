// LIBRARIES
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

// ADDRESSES
const uint16_t thisNode = 03; // Address of our actuator node in Octal format
const uint16_t otherNode = 00; // Address of base node

// GLOBAL VARIABLES
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
RF24Network network(radio); //create object to use nRF24L01 in mesh network

const unsigned long interval = 1000; // 1 second - how often to send 
unsigned long last_sent; // when did we last send?

// PIN CONNECTIONS
const int relayPin = 4; //relay on digital pin 4
const int ledPin = 8; //LED on digital pin 8
const int acsPin = A0; //ACS712 on analog pin 0

int relayStatus = 0; //store value of relay here
int acsValue = 0; //store value of current sensor here

void setup(void)
{
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication and control
  network.begin(90, thisNode); //(channel, node address)

  pinMode(relayPin, OUTPUT); // set relay as output 
  pinMode(ledPin, OUTPUT); // set LED as output
  pinMode(acsPin, INPUT); // set current sensor as input
}

void loop(void){
  digitalWrite(ledPin, HIGH); //turn on led to show sensor node is turned ON
  network.update();                  // Check the network regularly
  acsValue = analogRead(acsPin);

  //=====RECEVING=====//
  while ( network.available() ) {     // Is there anything ready for us?
    RF24NetworkHeader header;        // If so, grab it and print it out
    int payload;
    network.read(header,&payload,sizeof(payload));

    if(payload == 1)
    {
      // trigger the relay once a payload is received.
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
  }
  
  //=====SENDING=====//
  unsigned long now = millis(); //get the current time in ms
  if (now - last_sent >= interval) //if 1 second passed already, we can send
  {
    last_sent = now; //store current time in ms
    RF24NetworkHeader header1(otherNode); //(what node to sent to)
    bool ok = network.write(header1, &acsValue, sizeof(acsValue));
  }
}

