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

const unsigned long interval = 1000; // 1 second - how often to send 
unsigned long last_sent; // when did we last send?

void setup() {
  Serial.begin(115200);
  Serial.println("RF24Network/examples/helloworld_rx/");
  
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication and control
  network.begin(90, thisNode); //(channel, node address)
}

void loop() {
  //initialize variables
  int sensorNode01 = 0;
  int sensorNode011 = 0;
  int actuatorNode03 = 0;
  int remoteNode = 0;
    
  network.update(); //check the network regularly
  // ======RECEIVING======= //
  while(network.available()) //is there any incoming data?
  {
    RF24NetworkHeader header;
    unsigned long incomingData; // initialize variable for incoming data
    network.read (header, &incomingData, sizeof(incomingData)); //Read the incoming data
    if(header.from_node == 01)
    {
      sensorNode01 = incomingData;
      Serial.print("SensorNode01 : ");
      Serial.println(sensorNode01);
    }
    if(header.from_node == 011)
    {
      sensorNode011 = incomingData;
      Serial.print("SensorNode011: ");
      Serial.println(sensorNode011);
    }
    if(header.from_node == 03)
    {
      actuatorNode03 = incomingData;
      Serial.print("ActNode03 Current: ");
      Serial.println(actuatorNode03);
    }
    if(header.from_node == 02)
    {
      remoteNode = incomingData;
      Serial.print("Trigger Actuator Node: ");
      Serial.println(remoteNode);
    }

    // ======SENDING======= //

    unsigned long now = millis(); //get the current time in ms
    if (now - last_sent >= interval) //if 1 second passed already, we can send
    {
      last_sent = now; //store current time in ms
      //=====CHANGE CODE HERE=====//
      int payload;
      if (remoteNode == 1)
      {
        payload = 1;
      }
      else
      {
        payload = 0;
      }
      //=========================//
      RF24NetworkHeader header1(node03); //(what node to sent to)
      bool ok1 = network.write(header1, &payload, sizeof(payload));

      //=====CHANGE CODE HERE=====//
      if (remoteNode == 2)
      {
        payload = 1;
      }
      else
      {
        payload = 0;
      }
      //=========================//
      RF24NetworkHeader header2(node03); //(what node to sent to)
      bool ok2 = network.write(header2, &payload, sizeof(payload));

      //=====CHANGE CODE HERE=====//
      if (remoteNode == 3)
      {
        payload = 1;
      }
      else
      {
        payload = 0;
      }
      //=========================//
      RF24NetworkHeader header3(node03); //(what node to sent to)
      bool ok3 = network.write(header3, &payload, sizeof(payload));

      //=====CHANGE CODE HERE=====//
      if (remoteNode == 4)
      {
        payload = 1;
      }
      else
      {
        payload = 0;
      }
      //=========================//
      RF24NetworkHeader header4(node03); //(what node to sent to)
      bool ok4 = network.write(header4, &payload, sizeof(payload));

      //=====CHANGE CODE HERE=====//
      if (remoteNode == 5)
      {
        payload = 1;
      }
      else
      {
        payload = 0;
      }
      //=========================//
      RF24NetworkHeader header5(node03); //(what node to sent to)
      bool ok5 = network.write(header5, &payload, sizeof(payload));
    }
  }
}
