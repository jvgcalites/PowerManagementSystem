// LIBRARIES
#include <RF24Network.h> //Library for networking nRF24L01s
#include <RF24.h> //Library for nRF24L01
#include <SPI.h> //nRF24L01 uses SPI communication

// ADDRESSES
const uint16_t thisNode = 01; // Address of our node in Octal format
const uint16_t otherNode = 00; // Address of the other node in Octal format

// GLOBAL VARIABLES
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
RF24Network network(radio); //create object to use nRF24L01 in mesh network

const unsigned long interval = 1000; // 1 second - how often to send 
unsigned long last_sent; // when did we last send?

const int sensorPin = 4; // PIR sensor on digital pin 4
const int ledPin = 8; //LED on digital pin 8
int payload = 0; //initialize variable payload
int currentStatus = 0; //stores the current status of the sensor node; motion or no motion

void setup() {
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication and control
  network.begin(90, thisNode); //(channel, node address)

  pinMode(ledPin, OUTPUT); // set ditital pin 8 as output control status LED
  pinMode(sensorPin, INPUT); // set digital pin 4 as input PIR Sensor
}

void loop() {
  digitalWrite(ledPin, HIGH); //turn on led to show sensor node is turned ON
  network.update(); //check the network regularly
  payload = digitalRead(sensorPin);

  unsigned long now = millis(); //get the current time in ms
  if (now - last_sent >= interval) //if 1 second passed already, we can send
  {
    last_sent = now; //store current time in ms
    
    if (payload != currentStatus) //blink the led if motion status change.
    {
      digitalWrite(ledPin, LOW);
      delay(100);
      digitalWrite(ledPin, HIGH);
    }

    currentStatus = payload; //store current motion status 
    RF24NetworkHeader header(otherNode); //(what node to sent to)
    bool ok = network.write(header, &payload, sizeof(payload));
  }
  
}
