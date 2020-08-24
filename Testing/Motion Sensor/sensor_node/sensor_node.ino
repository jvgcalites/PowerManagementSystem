// LIBRARIES
#include <nRF24L01.h>
#include <RF24.h> //Library for nRF24L01
#include <SPI.h> //nRF24L01 uses SPI communication
#include <avr/sleep.h>
#include "PinChangeInterrupt.h"

// WRITE ADDRESSES TO ACTUATOR NODE(uncomment the one you need)

//// FOR SENSOR NODE 1
//const uint64_t wAddress = 0xB00B1E50C1LL; 

//// FOR SENSOR NODE 2
//const uint64_t wAddress = 0xB00B1E50C2LL; 

// FOR SENSOR NODE 3
const uint64_t wAddress = 0xB00B1E50C3LL; 

//// FOR SENSOR NODE 4
//const uint64_t wAddress = 0xB00B1E50C4LL; 


// NRF24L01 
RF24 radio(9, 10); //create object to control and communicate with nRF24L01
byte payload = 0; //initialize variable payload

// I/O PINS
#define sensorPin 4
const int ledPin = 8; //LED on digital pin 8
const int PCINT_DCPIN4 = 20; // Assigned PCINT for PD4 

// INTERRUPT STATUS
volatile int previousState;
volatile int currentState; 


void setup() {
    // setup NRF24L01
    SPI.begin(); //start SPI communication
    radio.begin(); //start nRF24L01 communication and control
    radio.openWritingPipe(wAddress); //open writing or transmit pipe
    radio.stopListening(); //go into transmit mode
    radio.powerDown();

    // setup pins
    pinMode(ledPin, OUTPUT); // set ditital pin 8 as output control status LED
    pinMode(sensorPin, INPUT); // set digital pin 4 as input PIR Sensor

    // LED blinks until PIR sensor is stabilized
    for( int i = 1; i <= 120; i++){  
        digitalWrite(ledPin, HIGH); 
        delay(100);         
        digitalWrite(ledPin, LOW); 
        delay(100); 
    }

    // Get the initial states
    currentState = digitalRead(sensorPin);
    previousState = currentState;

}

void loop() {

    // enter here when sensor node detects a motion
    if(currentState != previousState)
    {
        //change the motion status
        previousState = currentState;
        
        //send the motion status to the base node
        radio.powerUp();
        payload = (byte)currentState;
        radio.write(&payload, 1);
        radio.powerDown();

        //turn on the led for 1 second upon sending the motion status
        digitalWrite(ledPin, HIGH);
        delay(1000);
        digitalWrite(ledPin, LOW);
    }  

    delay(50);
    hibernate(); // go to sleep - call sleeping function
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
    sleep_mode();
    sleep_disable();
    //detachPCINT(PCINT_DCPIN4);
}

// END
