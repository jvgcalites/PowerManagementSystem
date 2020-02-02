// LIBRARIES
#include <RF24Network.h> //Library for networking nRF24L01s
#include <RF24.h> //Library for nRF24L01
#include <SPI.h> //nRF24L01 uses SPI communication

// ADDRESSES
const uint16_t thisNode = 011; // Address of our node in Octal format
const uint16_t otherNode = 00; // Address of the other node in Octal format

// GLOBAL VARIABLES
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
RF24Network network(radio); //create object to use nRF24L01 in mesh network

const int sensorPin = 4; // PIR sensor on digital pin 4
const int ledPin = 8; //LED on digital pin 8
int payload = 0; //initialize variable payload
bool justTurnedOn = true;
int currentStatus = 0; //stores the current status of the sensor node; motion or no motion
int previousStatus = 0; // stores the previous status of the sensor node

void setup() {
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication and control
  network.begin(90, thisNode); //(channel, node address)

  pinMode(ledPin, OUTPUT); // set ditital pin 8 as output control status LED
  pinMode(sensorPin, INPUT); // set digital pin 4 as input PIR Sensor
}

void loop() {
  network.update(); //check the network regularly

  if(justTurnedOn) //turn on the led for 1 second upon starting
  {
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
    justTurnedOn = false;
  }

  currentStatus = digitalRead(sensorPin);

  //check the previous status is different from the current status 
  if(currentStatus != previousStatus)
  {
    //change the motion status
    previousStatus = currentStatus;
    
    //send the motion status to the base node
    payload = currentStatus;
    RF24NetworkHeader header(otherNode); // (what node to sent to)
    bool ok = network.write(header, &payload, sizeof(payload));

    //turn on the led for 1 second upon sending the motion status
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
  }  
}
