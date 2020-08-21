// LIBRARIES
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

// NRF24L01
RF24 radio(9, 10); //create object to control and communicate with nRF24L01

// ADDRESSES
const uint64_t rAddress[] = {
                                0xF0F0F0F0B3LL, // base node
                                0xB00B1E50C1LL, // sensor node 1
                                0xB00B1E50C2LL, // sensor node 2
                                0xB00B1E50C3LL, // sensor node 3
                                0xB00B1E50C4LL  // sensor node 4
                            };  
const uint64_t wAddress = 0xF0F0F0F0A3LL;  // base node

// PIN CONNECTIONS
const int relayPin = 4; //relay on digital pin 4
const int ledPin = 8; //LED on digital pin 8
const int acsPin = A0; //ACS712 on analog pin 0

// ACS712 CURRENT SENSOR
int mVperAmp = 100; // use 185 for 10A Module, 100 for 20A Module, and 66 for 30A Module
double voltage = 0;
double vRMS = 0;
double ampsRMS = 0;
double acsThresholdValue = 0.15; //used for determining whether the device is on or off.

// INITIALIZE VARIABLES
int relayStatus = 0; //store value of relay here

void setup(void){
    // setup NRF24L01
    Serial.begin(115200);
    SPI.begin(); 
    radio.begin(); 
    radio.openReadingPipe(1,rAddress[1]); // sensor node 1
    radio.openReadingPipe(2,rAddress[2]); // sensor node 2
    radio.openReadingPipe(3,rAddress[3]); // sensor node 3
    radio.openReadingPipe(4,rAddress[4]); // sensor node 4
    radio.openReadingPipe(5,rAddress[0]); // base node
    radio.openWritingPipe(wAddress); 
    radio.startListening();                

    // setup i/o pins
    pinMode(relayPin, OUTPUT);
    pinMode(ledPin, OUTPUT); 
    pinMode(acsPin, INPUT); 

    // set LED on
    digitalWrite(ledPin, HIGH);
}

void loop(void){
    byte pipeNum = 0; //variable to hold which reading pipe sent data
    byte gotByte = 0; //used to store payload from transmit module
    
    if(radio.available(&pipeNum)) {
        radio.read( &gotByte, 1 );

        // if the data came from the base node
        if (pipeNum == 5){
            // if actuator node is triggered using remote node
            if(gotByte == 1){
                // trigger the relay once the payload is received.
                relayStatus = digitalRead(relayPin);
                if(relayStatus == HIGH){
                    digitalWrite(relayPin, LOW);
                }
                else {
                    digitalWrite(relayPin, HIGH);
                }

                blinkLed();
            }

            // if actuator node is triggered using sensor node
            if (gotByte == 2){
                // get AC current reading
                voltage = getVVP();
                vRMS = (voltage/2.0) *0.707;  //root 2 is 0.707
                ampsRMS = (vRMS * 1000)/mVperAmp;

                // if there is current flowing, the device is turned on. Thus, trigger the relay.
                if(ampsRMS > acsThresholdValue){
                    relayStatus = digitalRead(relayPin);
                    if(relayStatus == HIGH){
                        digitalWrite(relayPin, LOW);
                    }
                    else {
                        digitalWrite(relayPin, HIGH);
                    }
                    blinkLed();
                }
            }
        }
        else { // the data must be came from one of the sensor nodes

            // prepare the data to be sent
            byte data = (pipeNum * 10) + gotByte;

            // pass the data to the base node
            radio.stopListening();
            radio.write(&data, 1);
            radio.startListening();

            // indicate that the data has been sent
            blinkLed();
        }
    }
}

// This function calculates the voltage using the data from the current sensor
float getVVP(){
    float result;
    float acsValue;
    int maxValue = 0;
    int minValue = 1024;

    uint32_t start_time = millis();
    while((millis() - start_time) < 1000){ // get sample for 1 second
        acsValue = analogRead(acsPin);
        if(acsValue > maxValue){
            maxValue = acsValue;
        }
        if(acsValue < minValue){
            minValue = acsValue;
        }
    }                                   
    result = ((maxValue - minValue) * 5.0) / 1024;
    
    return result;
}

// This function blinks the led for 1/10 of a second
void blinkLed()
{
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
}


