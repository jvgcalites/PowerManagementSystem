// LIBRARIES
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>

// NRF24L01
RF24 radio(9, 10); //create object to control and communicate with nRF24L01

// ADDRESSES
const uint64_t rAddress[] = {   
                                0xF0F0F0F0A1LL, // camera node
                                0xF0F0F0F0A2LL, // remote node
                                0xF0F0F0F0A3LL, // actuator node 1
                            }; 
const uint64_t wAddress[] = {   
                                0xF0F0F0F0B1LL, // camera node
                                0xF0F0F0F0B2LL, // remote node
                                0xB00B1E50FFLL, // actuator node 1
                                0xF0F0F0F0B4LL, // actuator node 2
                                0xF0F0F0F0B5LL, // actuator node 3
                                0xF0F0F0F0B6LL, // actuator node 4
                            }; 

// --ACTUATOR NODE--
// PIN CONNECTIONS
const int relayPin = 4; //relay on digital pin 4
const int ledPin = 8; //LED on digital pin 8
const int acsPin = A0; //ACS712 on analog pin 0

// ACS712 CURRENT SENSOR
int mVperAmp = 100; // use 185 for 10A Module, 100 for 20A Module, and 66 for 30A Module
double voltage = 0;
double vRMS = 0;
double ampsRMS = 0;
double acsThresholdValue = 0.16; //used for determining whether the device is on or off.

// INITIALIZE VARIABLES
int relayStatus = 0; //store value of relay here
byte payload = 0;

const long minIdleTime = 120000; // if room idle time reached 2 mins, ask the camera for verification
unsigned long currentMillis; // time counter

// --SENSOR NODE--
int sn1 = 0; //store the latest data received from sensor node 1
unsigned long sn1_previousMillis; //store last time sensor node 1 sent 0;
unsigned long sn1_idleTime; //store the idle time of sensor node 1
int sn2 = 0;
unsigned long sn2_previousMillis;
unsigned long sn2_idleTime;
int sn3 = 0;
unsigned long sn3_previousMillis;
unsigned long sn3_idleTime;
int sn4 = 0;
unsigned long sn4_previousMillis;
unsigned long sn4_idleTime;

void setup(void){
    Serial.begin(115200);

    // setup NRF24L01
    SPI.begin(); //start SPI communication
    radio.begin(); 
    radio.openReadingPipe(1,rAddress[0]); // from camera node
    radio.openReadingPipe(2,rAddress[1]); // from remote node    
    radio.openReadingPipe(3,rAddress[2]); // from actuator node 1     
    radio.startListening(); 

    // setup i/o pins
    pinMode(relayPin, OUTPUT);
    pinMode(ledPin, OUTPUT); 
    pinMode(acsPin, INPUT); 

    // set LED on
    digitalWrite(ledPin, HIGH);
	
	// ask the camera if room is occupied
    // for some reason, the camera doesn't respond the first time it received a message
    radio.stopListening();
    radio.enableDynamicPayloads();
    const char text[] = "Occupied?";
    radio.openWritingPipe(wAddress[0]);
    radio.write(text, sizeof(text));
    Serial.print("To Camera: ");
    Serial.println(text);
    radio.disableDynamicPayloads();
    radio.startListening();
}

void loop(void){
    currentMillis = millis(); // get total running time starting from the time the program started
    byte pipeNum = 0; //variable to hold which reading pipe sent data
    byte gotByte = 0; //used to store payload from transmit module
  
    if (radio.available(&pipeNum)) {
        // check which pipe the data came from
        if (pipeNum == 1){  // if data came from camera node
            blinkLed();
            updateAllIdleTime();

            radio.enableDynamicPayloads(); // used to enable sending this kind of message
            // if camera responds, receive its message
            char receivedMessage[32] = {0};
            radio.read(receivedMessage, sizeof(receivedMessage));
            Serial.print("From Camera: ");
            Serial.println(receivedMessage);
            String stringMessage(receivedMessage);
            radio.disableDynamicPayloads();

            // if the camera did not detect any human presence, the room is empty
            if(stringMessage == "NO"){
                Serial.println("Sending signal to all actuator nodes to turn off");
                //turn off all actuator nodes
                payload = 2;
                sendData(3, payload); // actuator node 1
                sendData(4, payload); // actuator node 2
                sendData(5, payload); // actuator node 3
                sendData(6, payload); // actuator node 4

                // actuator node 5
                // get AC current reading
                voltage = getVVP();
                vRMS = (voltage/2.0) *0.707;  //root 2 is 0.707
                ampsRMS = (vRMS * 1000)/mVperAmp;

                // if there is current flowing, the device is turned on. Thus, trigger the relay.
                if(ampsRMS > acsThresholdValue){
                    relayStatus = digitalRead(relayPin);
                    if(relayStatus == HIGH){
                        digitalWrite(relayPin, LOW);
                    } else {
                        digitalWrite(relayPin, HIGH);
                    }
                }
            }
        } else if (pipeNum == 2){ // if data came from remote node
            blinkLed();
            updateAllIdleTime();
            
            //read one byte of data and store it in gotByte variable
            radio.read( &gotByte, 1 );

            Serial.print("Remote node sent :");
            Serial.println(gotByte);

            payload = 1;
            if (gotByte == 1){
                Serial.println("Sending signal to actuator node 1 to turn off");
                sendData(3, payload); // actuator node 1
            } else if (gotByte == 2){
                Serial.println("Sending signal to actuator node 2 to turn off");
                sendData(4, payload); // actuator node 2
            } else if (gotByte == 3){
                Serial.println("Sending signal to actuator node 3 to turn off");
                sendData(5, payload); // actuator node 3
            } else if (gotByte == 4){
                Serial.println("Sending signal to actuator node 4 to turn off");
                sendData(6, payload); // actuator node 4
            } else if (gotByte == 5){
                Serial.println("This actuator node will turn off");
                // trigger the relay of this node.
                relayStatus = digitalRead(relayPin);
                if(relayStatus == HIGH){
                    digitalWrite(relayPin, LOW);
                } else {
                    digitalWrite(relayPin, HIGH);
                }
            } else {
                // It should not reach here, in theory
            }
        } else if (pipeNum == 3){ // if data came from actuator node 1
            //read one byte of data and store it in gotByte variable
            radio.read( &gotByte, 1 );

            Serial.print("Received: ");
            Serial.println(gotByte);

            // possible contents of payload - 10, 11, 20, 21, 30, 31, 40, 41
            int sensorNodeId = gotByte / 10;
            int sensorNodeStatus = gotByte % 10;

            // change the status of the specific sensor node
            // Sensor Node 1
            if(sensorNodeId == 1){
                sn1 = sensorNodeStatus;
                if(sensorNodeStatus == 0){
                    sn1_idleTime = updateIdleTime(sn1_previousMillis);
                } else { 
                    sn1_previousMillis = currentMillis;
                    sn1_idleTime = updateIdleTime(sn1_previousMillis);
                }
            } else { 
                sn1_idleTime = updateIdleTime(sn1_previousMillis);
            }

            // Sensor Node 2
            if(sensorNodeId == 2){ 
                sn2 = sensorNodeStatus;
                if(sensorNodeStatus == 0){ 
                    sn2_idleTime = updateIdleTime(sn2_previousMillis);
                } else { 
                    sn2_previousMillis = currentMillis;
                    sn2_idleTime = updateIdleTime(sn2_previousMillis);
                }
            } else { 
                sn2_idleTime = updateIdleTime(sn2_previousMillis);
            }

            // Sensor Node 3
            if(sensorNodeId == 3){ 
                sn3 = sensorNodeStatus;
                if(sensorNodeStatus == 0){ 
                    sn3_idleTime = updateIdleTime(sn3_previousMillis);
                } else { 
                    sn3_previousMillis = currentMillis;
                    sn3_idleTime = updateIdleTime(sn3_previousMillis);
                }
            } else { 
                sn3_idleTime = updateIdleTime(sn3_previousMillis);
            }

            // Sensor Node 4
            if(sensorNodeId == 4) {
                sn4 = sensorNodeStatus;
                if(sensorNodeStatus == 0){ 
                    sn4_idleTime = updateIdleTime(sn4_previousMillis);
                } else {
                    sn4_previousMillis = currentMillis;
                    sn4_idleTime = updateIdleTime(sn4_previousMillis);
                }
            } else{ // if received data is from other node
                sn4_idleTime = updateIdleTime(sn4_previousMillis);
            } 
        } else {
            // It should not reach here, in theory
        }
    } else { // if no data is received, still update the idle time of the sensor nodes
        updateAllIdleTime();
    }

    Serial.println("=========================================================");
    Serial.print("Timer :");
    Serial.println(currentMillis);
    printStatus("SN1", sn1_idleTime, sn1);
    printStatus("SN2", sn2_idleTime, sn2);
    printStatus("SN3", sn3_idleTime, sn3);
    printStatus("SN4", sn4_idleTime, sn4);
    
    if(sn1_idleTime > minIdleTime && sn2_idleTime > minIdleTime && sn3_idleTime > minIdleTime && sn4_idleTime > minIdleTime){
        // ask the camera if room is occupied
        radio.stopListening();
        radio.enableDynamicPayloads();
        const char text[] = "Occupied?";
        radio.openWritingPipe(wAddress[0]);
        radio.write(text, sizeof(text));
        Serial.print("To Camera: ");
        Serial.println(text);
        radio.disableDynamicPayloads();
        radio.startListening();

        // reset all sensor node's the idle time
        sn1_previousMillis = currentMillis;
        sn2_previousMillis = currentMillis;
        sn3_previousMillis = currentMillis;
        sn4_previousMillis = currentMillis;
    }

    delay(1000);
}

float getVVP(){
    float result;
    float acsValue;
    int maxValue = 0;
    int minValue = 1024;

    uint32_t start_time = millis();
    while((millis() - start_time) < 1000) // get sample for 1 second
    {
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

void printStatus(String sensorNode, unsigned long idleTime, int data)
{
  Serial.print(sensorNode);
  Serial.print("_IdleTime: ");
  Serial.print(idleTime);
  Serial.print(" | Latest Data Received: ");
  Serial.println(data);
}

void blinkLed(){
    //blink led showing data sent status
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
}

unsigned long updateIdleTime(unsigned long prevMillis){
  unsigned long idleTime = 0;
  idleTime = currentMillis - prevMillis; //update the idle time
  return idleTime;
}

void updateAllIdleTime(){
    // Sensor Node 1
    if(sn1 == 0){ //check if latest received data is no motion
        sn1_idleTime = updateIdleTime(sn1_previousMillis);
    }
    else{ //if latest received data is 1
        sn1_previousMillis = currentMillis;
        sn1_idleTime = updateIdleTime(sn1_previousMillis);
    }

    // Sensor Node 2
    if(sn2 == 0){ 
        sn2_idleTime = updateIdleTime(sn2_previousMillis);
    }
    else{
        sn2_previousMillis = currentMillis;
        sn2_idleTime = updateIdleTime(sn2_previousMillis);
    }
    
    // Sensor Node 3
    if(sn3 == 0){
        sn3_idleTime = updateIdleTime(sn3_previousMillis);
    }
    else{ 
        sn3_previousMillis = currentMillis;
        sn3_idleTime = updateIdleTime(sn3_previousMillis);
    }

    // Sensor Node 4
    if(sn4 == 0){ 
        sn4_idleTime = updateIdleTime(sn4_previousMillis);
    }
    else{
        sn4_previousMillis = currentMillis;
        sn4_idleTime = updateIdleTime(sn4_previousMillis);
    }
}

void sendData(byte pipeNumber, byte payload){
    radio.stopListening();
    radio.openWritingPipe(wAddress[pipeNumber - 1]);
    radio.write(&payload, 1);
    radio.startListening();
}
