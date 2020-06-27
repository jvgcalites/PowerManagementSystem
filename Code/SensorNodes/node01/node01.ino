// LIBRARIES
#include <RF24Network.h> //Library for networking nRF24L01s
#include <RF24.h> //Library for nRF24L01
#include <SPI.h> //nRF24L01 uses SPI communication
#include <avr/sleep.h>
#include "PinChangeInterrupt.h"

// ADDRESSES
const uint16_t thisNode = 01; // Address of our node in Octal format
const uint16_t otherNode = 00; // Address of the other node in Octal format

// NRF24L01 
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
RF24Network network(radio); //create object to use nRF24L01 in mesh network
int payload = 0; //initialize variable payload

// I/O
// const int sensorPin = 4; // PIR sensor on digital pin 4
#define sensorPin 4
const int ledPin = 8; //LED on digital pin 8
const int PCINT_DCPIN4 = 20; // Assigned PCINT for PD4 

// INTERRUPT STATUS
volatile int previousState = 1;
volatile int currentState = 0; 


void setup() {
  SPI.begin(); //start SPI communication
  radio.begin(); //start nRF24L01 communication and control
  network.begin(90, thisNode); //(channel, node address)
  
  Serial.begin(115200);     // initialize serial communication only for debugging purpose
  Serial.println("Warming up... wait for a min...");
  
  pinMode(ledPin, OUTPUT); // set ditital pin 8 as output control status LED
  pinMode(sensorPin, INPUT); // set digital pin 4 as input PIR Sensor

  // LED blinks until PIR sensor is stabilized
  for( int i = 1; i <= 120; i++){  
    digitalWrite(ledPin, HIGH); 
    delay(100);         
    digitalWrite(ledPin, LOW); 
    delay(100); 
  }
  Serial.println("Ready");     // enable only for debugging purpose
}


void wakeUpNow(){
  // change motion sensor state from ON to OFF and vice versa.
  currentState = !previousState;
}

void hibernate(){
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // lowest power consumption mode
   ADCSRA &= ~(1 << 7); // disable ADC
   sleep_enable();
   sleep_bod_disable();
   attachPCINT(20, wakeUpNow, CHANGE);
//   for (int i = 0; i < 20; i++) {
//    if(i != 4)//  because the LED is connected to digital pin 4
//    pinMode(i, INPUT);
//  }
  sleep_mode();
  sleep_disable();
  //detachPCINT(PCINT_DCPIN4);
}

void loop() {
  network.update(); //check the network regularly

  //check the previous status is different from the current status 
  if(currentState != previousState)
  {
    Serial.print("CurrentState: ");
    Serial.println(currentState);
    
    //change the motion status
    previousState = currentState;
    
    //send the motion status to the base node
    payload = currentState;
    RF24NetworkHeader header(otherNode); // (what node to sent to)
    bool ok = network.write(header, &payload, sizeof(payload));

    //turn on the led for 1 second upon sending the motion status
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
  }  

  delay(50);
  hibernate(); // go to sleep - call sleeping function
}

// END




