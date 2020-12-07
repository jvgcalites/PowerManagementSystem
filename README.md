# Power Management System
Wireless Sensor and Actuator Network-based Power Management System using Raspberry Pi and Image Processing for Smart Classroom
### What is this?
This repository contains the PCB design boards and source codes that was used to create the working prototype of the system.
### What is Power Management?
Power Management is a feature commonly seen in electronic devices that turns off the power or switches the system to a low power state when inactive. In this case, this feature is applied in a classroom, where each equipment inside it automatically turns off when the room is unoccupied. 
### How does the system determine whether room is occupied or not?
The system utilize multiple motion sensors to cover the room and a camera that uses human detection using raspberry pi.
### Why apply WSAN?
The system is composed of nodes:
* Sensor node - captures motion using motion sensors
* Camera node - captures image using pi camera and applying human detection using raspberry pi
* Actuator Node - serve as a power strip for each equipment that can be controlled automatically with a relay
* Remote Node - use for remote control of actuator nodes

Each node will be placed in different location in the room, resulting them to be far from each other. So, each node uses NRF24L01 transceiver modules to connect with each other in a wireless network.
 


 
