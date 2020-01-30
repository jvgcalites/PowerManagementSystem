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

const long minIdleTime = 60000; //after 60 seconds, turn off the devices
unsigned long sn1_previousMillis; //store last time sensor node 1 sent 0;
unsigned long currentMillis;
unsigned long sn1_idleTime; //store the idle time of sensor node 1
int incomingData = 0; // initialize variable for incoming data


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

  currentMillis = millis();
  
  network.update(); //check the network regularly
  // ======RECEIVING======= //
  if(network.available()) //is there any incoming data?
  {
    RF24NetworkHeader header;
    network.read(header, &incomingData, sizeof(incomingData)); //Read the incoming data
    
    //+++++FROM SENSOR NODE 1++++//
    if(header.from_node == 01) //if received data is from sensor node 1
    {
      sensorNode01 = incomingData;
      if(incomingData == 0) //check if received data is no motion
      {
        sn1_idleTime = updateIdleTime(currentMillis, sn1_previousMillis);
      }
      else //if received data is 1
      {
        sn1_previousMillis = currentMillis;
        sn1_idleTime = updateIdleTime(currentMillis, sn1_previousMillis);
      }
    }
    else // if received data is from other node
    {
      sn1_idleTime = updateIdleTime(currentMillis, sn1_previousMillis);
    }
    //++++++++++++++++++++++++++//
    
    printStatus(sn1_idleTime, sensorNode01);
    delay(1000);
  }
  else
  {
    if(incomingData == 0) //check if received data is no motion
    {
      sn1_idleTime = updateIdleTime(currentMillis, sn1_previousMillis);
    }
    else //if received data is 1
    {
      sn1_previousMillis = currentMillis;
      sn1_idleTime = updateIdleTime(currentMillis, sn1_previousMillis);
    }
    sensorNode01 = incomingData;
    printStatus(sn1_idleTime, sensorNode01);
    delay(1000);
  }

  if(sn1_idleTime > minIdleTime)
  {
    int payload = 2;
    RF24NetworkHeader header1(node03); //(what node to sent to)
    bool ok = network.write(header1, &payload, sizeof(payload));
  }
}

void printStatus(unsigned long idleTime, int data)
{
  Serial.print("SN1_IdleTime: ");
  Serial.print(idleTime);
  Serial.print(" | Latest Data Received: ");
  Serial.println(data);
}

unsigned long updateIdleTime(unsigned long currMillis, unsigned long prevMillis)
{
  unsigned long idleTime = 0;
  idleTime = currMillis - prevMillis; //update the idle time
  return idleTime;
}

