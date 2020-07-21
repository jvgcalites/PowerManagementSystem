// LIBRARIES
#include <RF24Network.h> //Library for networking nRF24L01s
#include <RF24.h> //Library for nRF24L01
#include <SPI.h> //nRF24L01 uses SPI communication

// ADDRESSES
const uint16_t thisNode_addr = 02; // Address of our remote node in Octal format
const uint16_t actuatorNode01_addr = 03;
const uint16_t actuatorNode02_addr = 03;
const uint16_t actuatorNode03_addr = 03;
const uint16_t actuatorNode04_addr = 03;
const uint16_t actuatorNode05_addr = 03;
const uint16_t baseNode_addr = 00; 

// GLOBAL VARIABLES
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
RF24Network network(radio); //create object to use nRF24L01 in mesh network

const int ledPin = 2; //LED on digital pin 2
bool justTurnedOn = true;
int payload = 1; //payload to sent to actuator nodes

void setup() {
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication and control
  network.begin(90, thisNode_addr); //(channel, node address)

  pinMode(ledPin, OUTPUT); // set digital pin 8 as output control status LED
}

void loop() {
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
    RF24NetworkHeader header(actuatorNode01_addr);
    bool ok1 = network.write(header, &payload, sizeof(payload));
    blinkLed();
  }
  else if(digitalRead(4) == LOW)
  {
    RF24NetworkHeader header(actuatorNode02_addr);
    bool ok2 = network.write(header, &payload, sizeof(payload));
    blinkLed();
  }
  else if(digitalRead(6) == LOW)
  {
    RF24NetworkHeader header(actuatorNode03_addr);
    bool ok3 = network.write(header, &payload, sizeof(payload));
    blinkLed();
  }
  else if(digitalRead(7) == LOW)
  {
    RF24NetworkHeader header(actuatorNode04_addr); 
    bool ok4 = network.write(header, &payload, sizeof(payload));
    blinkLed();
  }
  else if(digitalRead(8) == LOW)
  {
    RF24NetworkHeader header(actuatorNode05_addr);
    bool ok5 = network.write(header, &payload, sizeof(payload));
    blinkLed();
  }
  else
  {
    payload = 1;
  }
  
  delay(500);
}

void blinkLed()
{
  //blink led showing data sent status
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
}

