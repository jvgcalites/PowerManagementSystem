// LIBRARIES
#include <RF24Network.h> //Library for networking nRF24L01s
#include <RF24.h> //Library for nRF24L01
#include <SPI.h> //nRF24L01 uses SPI communication

// ADDRESSES
const uint16_t thisNode = 00; // Address of our node in Octal format
const uint16_t node03 = 03; // Address of actuator node

// GLOBAL VARIABLES
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
RF24Network network(radio); //create object to use nRF24L01 in mesh network


int incomingData = 0; // initialize variable for incoming data

int sensorNode01 = 0; //store the latest data received from sensor node 1
unsigned long sn1_previousMillis; //store last time sensor node 1 sent 0;
unsigned long sn1_idleTime; //store the idle time of sensor node 1
int sensorNode011 = 0;
unsigned long sn2_previousMillis;
unsigned long sn2_idleTime;
int actuatorNode03 = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("TEST STARTS. YOU SHOULD RECEIVE RAW VALUES FROM CURRENT SENSOR");
  
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication and control
  network.begin(90, thisNode); //(channel, node address)
}

void loop() {
  network.update(); //check the network regularly
  
  // ======RECEIVING======= //
  if(network.available()) //is there any incoming data?
  {
    RF24NetworkHeader header;
    network.read(header, &incomingData, sizeof(incomingData)); //Read the incoming data
    Serial.println(incomingData);
  }

  delay(1000);
}

