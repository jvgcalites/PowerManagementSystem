// LIBRARIES
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

// ADDRESSES
const uint16_t thisNode = 03; // Address of our actuator node in Octal format
const uint16_t otherNode = 00; // Address of base node

// OBJECTS
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
RF24Network network(radio); //create object to use nRF24L01 in mesh network

// PIN CONNECTIONS
const int relayPin = 4; //relay on digital pin 4
const int ledPin = 8; //LED on digital pin 8
const int acsPin = A0; //ACS712 on analog pin 0

// INITIALIZE VARIABLES
int relayStatus = 0; //store value of relay here
bool justTurnedOn = true;

// Variables used for ACS712 Current Sensor
int acsValue = 0; //store value of current sensor here
int acsLimit = 10; //used for determining whether the device is on or off.
const float VCC = 5.0; // supply voltage is from 4.5 to 5.5V. Normally 5V
float cutOffLimit = 0.001; //set the current which bellow that value. 
float sensitivity = 0.100; // set for ACS712 20A
const float QOV = 0.5 *VCC;
float voltage; //internal variable for voltage.


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

    if(payload == 1) // actuator node is triggered using remote node
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

    if (payload == 2) // actuator node is triggered using sensor node
    {
      //check if there is current flowing through the current sensor
      acsValue = analogRead(acsPin);
      float rawVoltage = (5.0/1023.0)*acsValue; //read the voltage from sensor
      voltage = rawVoltage - QOV + 0.012; //0.000 is a value to make voltage zero when there is no current
      float current = voltage / sensitivity;

      //send the current reading to the base node 
      RF24NetworkHeader header1(otherNode); // (what node to sent to)
      bool ok = network.write(header1, &current, sizeof(current));
      
      if(current > cutOffLimit) // If there is current flowing, the device is turned on. Thus, trigger the relay.
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

