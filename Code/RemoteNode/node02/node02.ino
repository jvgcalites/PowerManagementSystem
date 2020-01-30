// LIBRARIES
#include <RF24Network.h> //Library for networking nRF24L01s
#include <RF24.h> //Library for nRF24L01
#include <SPI.h> //nRF24L01 uses SPI communication

// ADDRESSES
const uint16_t thisNode = 02; // Address of our remote node in Octal format
const uint16_t node03 = 00; // Address of actuator node

// GLOBAL VARIABLES
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
RF24Network network(radio); //create object to use nRF24L01 in mesh network

const int ledPin = 2; //LED on digital pin 2
bool justTurnedOn = true;

void setup() {
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication and control
  network.begin(90, thisNode); //(channel, node address)

  pinMode(ledPin, OUTPUT); // set ditital pin 8 as output control status LED
}

void loop() {
  int payload = 1; //initialize variable payload
  if(justTurnedOn) //turn on the led for 1 second upon starting
  {
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
    justTurnedOn = false;
  }
  
  network.update(); //check the network regularly

  if(digitalRead(3) == LOW)
  {
    RF24NetworkHeader header(node03); //(what node to sent to)
    bool ok1 = network.write(header, &payload, sizeof(payload));
    blinkLed();
  }
  else if(digitalRead(4) == LOW)
  {
    RF24NetworkHeader header(node03); //(what node to sent to)
    bool ok2 = network.write(header, &payload, sizeof(payload));
    blinkLed();
  }
  else if(digitalRead(6) == LOW)
  {
    RF24NetworkHeader header(node03); //(what node to sent to)
    bool ok3 = network.write(header, &payload, sizeof(payload));
    blinkLed();
  }
  else if(digitalRead(7) == LOW)
  {
    RF24NetworkHeader header(node03); //(what node to sent to)
    bool ok4 = network.write(header, &payload, sizeof(payload));
    blinkLed();
  }
  else if(digitalRead(8) == LOW)
  {
    RF24NetworkHeader header(node03); //(what node to sent to)
    bool ok5 = network.write(header, &payload, sizeof(payload));
    blinkLed();
  }
  else
  {
    payload = 1;
  }
}

void blinkLed()
{
  //blink led showing data sent status
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
}

