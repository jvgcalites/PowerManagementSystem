// LIBRARIES
#include <RF24Network.h> //Library for networking nRF24L01s
#include <RF24.h> //Library for nRF24L01
#include <SPI.h> //nRF24L01 uses SPI communication

// ADDRESSES
const uint16_t thisNode = 02; // Address of our remote node in Octal format
const uint16_t node00 = 00; // Address of actuator node

// GLOBAL VARIABLES
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
RF24Network network(radio); //create object to use nRF24L01 in mesh network

const unsigned long interval = 1000; // 1 second - how often to send 
unsigned long last_sent; // when did we last send?

const int ledPin = 2; //LED on digital pin 2
int payload = 0; //initialize variable payload

void setup() {
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication and control
  network.begin(90, thisNode); //(channel, node address)

  pinMode(ledPin, OUTPUT); // set ditital pin 8 as output control status LED
}

void loop() {

  digitalWrite(ledPin, HIGH); //turn on led to show sensor node is turned ON
  network.update(); //check the network regularly

  unsigned long now = millis(); //get the current time in ms
  if (now - last_sent >= interval) //if 1 second passed already, we can send
  {
    last_sent = now; //store current time in ms

    if(digitalRead(3) == LOW)
    {
      payload = 1;
    }
    else if(digitalRead(4) == LOW)
    {
      payload = 2;
    }
    else if(digitalRead(6) == LOW)
    {
      payload = 3;
    }
    else if(digitalRead(7) == LOW)
    {
      payload = 4;
    }
    else if(digitalRead(8) == LOW)
    {
      payload = 5;
    }
    else
    {
      payload = 0;
    }

    RF24NetworkHeader header(node00); //(what node to sent to)
    bool ok = network.write(header, &payload, sizeof(payload));

    //blink led showing data sent status
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
  }
}
