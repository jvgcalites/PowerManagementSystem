// LIBRARIES
#include <nRF24L01.h>
#include <RF24.h> //Library for nRF24L01
#include <SPI.h> //nRF24L01 uses SPI communication
#include <avr/sleep.h>
#include "PinChangeInterrupt.h"

// ADDRESSES
const uint64_t wAddress = 0xF0F0F0F0A2LL; // pipe address to base node

// NRF24L01
RF24 radio(9, 10); //create object to control and communicate with nRF24L01

// GLOBAL VARIABLES
const int ledPin = 2; //LED on digital pin 2
byte payload = 1; //payload to sent to actuator nodes

void setup() {
    // setup NRF24L01
    SPI.begin(); //start SPI communication
    radio.begin(); //start nRF24L01 communication and control
    radio.openWritingPipe(wAddress); //open writing or transmit pipe
    radio.stopListening(); //go into transmit mode
    radio.powerDown();

    // i/o pins
    pinMode(ledPin, OUTPUT); // set digital pin 8 as output control status LED

    // turn on the LED for 1 second upon start
    digitalWrite(ledPin, HIGH);
    delay(1000);
    digitalWrite(ledPin, LOW);
}

void wakeUpNow(){
    blinkLed();
}

void hibernate(){
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // lowest power consumption mode
    ADCSRA &= ~(1 << 7); // disable ADC
    sleep_enable();
    sleep_bod_disable();
    attachPCINT(19, wakeUpNow, RISING);
    attachPCINT(20, wakeUpNow, RISING);
    attachPCINT(22, wakeUpNow, RISING);
    attachPCINT(23, wakeUpNow, RISING);
    attachPCINT(0, wakeUpNow, RISING);
    sleep_mode();
    sleep_disable();
    //detachPCINT(PCINT_DCPIN4);
}

void loop() {  

    if(digitalRead(3) == LOW){
        payload = 1;
        radio.powerUp();
        radio.write(&payload, 1);
        radio.powerDown();
    }
    else if(digitalRead(4) == LOW){
        payload = 2;
        radio.powerUp();
        radio.write(&payload, 1);
        radio.powerDown();
    }
    else if(digitalRead(6) == LOW){
        payload = 3;
        radio.powerUp();
        radio.write(&payload, 1);
        radio.powerDown();
    }
    else if(digitalRead(7) == LOW){
        payload = 4;
        radio.powerUp();
        radio.write(&payload, 1);
        radio.powerDown();
    }
    else if(digitalRead(8) == LOW){
        payload = 5;
        radio.powerUp();
        radio.write(&payload, 1);
        radio.powerDown();
    }
    else{
        // It should not reach here, in theory
    }
    
    delay(500);
    hibernate(); // go to sleep - call sleeping function
}

void blinkLed(){
    //blink led showing data sent status
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
}
