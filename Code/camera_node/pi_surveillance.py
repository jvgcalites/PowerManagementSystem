#python pi_surveillance.py --conf conf.json

from __future__ import print_function
from picamera.array import PiRGBArray
from picamera import PiCamera
import argparse
import warnings
import datetime
import imutils
import json
import time
import cv2
import RPi.GPIO as GPIO
from imutils.object_detection import non_max_suppression
import numpy as np
# NRFL01 libraries
from lib_nrf24 import NRF24
import spidev

GPIO.setmode(GPIO.BCM)

# NRF24L01 setup
pipes = [[0xF0, 0xF0, 0xF0, 0xF0, 0xA1], [0xF0, 0xF0, 0xF0, 0xF0, 0xB1]]

radio = NRF24(GPIO, spidev.SpiDev())
radio.begin(0, 5)

radio.setPayloadSize(32)
#radio.setChannel(0x76)
radio.setDataRate(NRF24.BR_1MBPS)
radio.setPALevel(NRF24.PA_MIN)

radio.setAutoAck(True)
radio.enableDynamicPayloads()
radio.enableAckPayload()

radio.openWritingPipe(pipes[0])
radio.openReadingPipe(1, pipes[1])
radio.printDetails()
# end

ap = argparse.ArgumentParser()
ap.add_argument("-c", "--conf", required=True,
    help="path to the JSON configuration file")
args = vars(ap.parse_args())
warnings.filterwarnings("ignore")
conf = json.load(open(args["conf"]))
client = None

# Camera Setup
camera = PiCamera()
camera.resolution = tuple(conf["resolution"])
camera.framerate = conf["fps"]
rawCapture = PiRGBArray(camera, size=tuple(conf["resolution"]))

# HOG descriptor
hog = cv2.HOGDescriptor()
hog.setSVMDetector(cv2.HOGDescriptor_getDefaultPeopleDetector())

# Setup Pins
pin = 18
GPIO.setmode(GPIO.BCM)
GPIO.setup(pin, GPIO.OUT, initial=0)

print("[INFO] warming up...")
time.sleep(conf["camera_warmup_time"])
avg = None
lastUploaded = datetime.datetime.now()
motionCounter = 0

for f in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
    radio.startListening()
    
    start = time.time()
    
    # this loop will break once message is received
    while not radio.available(0):
        time.sleep(1/100)
        
    # if message is received, get it
    receivedMessage = []
    radio.read(receivedMessage, radio.getDynamicPayloadSize())
    
    # decode the message
    string = ""
    for n in receivedMessage:
        if (n >= 32 and n <= 126):
            string += chr(n)
            
    print(string)
            
    # respond if message came from the base node
    if string == "Occupied?":
        # run the image processing
        frame = f.array
        timestamp = datetime.datetime.now()
        text = "Unoccupied"
        frame = imutils.resize(frame, width=500)
        orig = frame.copy()

        (rects, weights) = hog.detectMultiScale(frame, winStride=(4, 4),
            padding=(8, 8), scale=1.05)


        # convert image to grayscale 
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        gray = cv2.GaussianBlur(gray, (21, 21), 0)

        if avg is None:
            print("[INFO] starting background model...")
            avg = gray.copy().astype("float")
            rawCapture.truncate(0)
            continue

        cv2.accumulateWeighted(gray, avg, 0.5)
        frameDelta = cv2.absdiff(gray, cv2.convertScaleAbs(avg))

        thresh = cv2.threshold(frameDelta, conf["delta_thresh"], 255,
            cv2.THRESH_BINARY)[1]
        thresh = cv2.dilate(thresh, None, iterations=2)
        cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
            cv2.CHAIN_APPROX_SIMPLE)
        cnts = cnts[0] if imutils.is_cv2() else cnts[1]



        # loop over the contours
        for c in cnts:
           
            if cv2.contourArea(c) < conf["min_area"]:
                continue

            for (x, y, w, h) in rects:
                cv2.rectangle(orig, (x, y), (x + w, y + h), (0, 0, 255), 2)
     
          
            rects = np.array([[x, y, x + w, y + h] for (x, y, w, h) in rects])
            pick = non_max_suppression(rects, probs=None, overlapThresh=0.65)
     
            for (xA, yA, xB, yB) in pick:
                cv2.rectangle(frame, (xA, yA), (xB, yB), (0, 255, 0), 2)

            text = "Occupied"

        ts = timestamp.strftime("%A %d %B %Y %I:%M:%S%p")
        cv2.putText(frame, "Room Status: {}".format(text), (10, 20),
            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
        cv2.putText(frame, ts, (10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX,
            0.35, (0, 0, 255), 1)
        
        # save the image
        cv2.imwrite("capturedImages/{}.png".format(ts), frame)

        # check if the room is occupied
        if text == "Occupied":
            # create the message
            message = list("YES")
            while len(message) < 32:
                message.append(0)
                
            # stop listening before sending the message
            radio.stopListening()
            
            #send the message
            radio.write(message)
            print(message)

            #Activate pin
            #Dito mo ayusin yung nrf
            GPIO.output(pin, GPIO.HIGH)
            
            # check to see if enough time has passed between uploads
            if (timestamp - lastUploaded).seconds >= conf["min_upload_seconds"]:
                
                motionCounter += 1

                if motionCounter >= conf["min_motion_frames"]:
                   
                    lastUploaded = timestamp
                    motionCounter = 0


        #the room is not occupied
        else:
            motionCounter = 0
            
            # create the message
            message = list("NO")
            while len(message) < 32:
                message.append(0)
                
            # stop listening before sending the message
            radio.stopListening()
            
            #send the message
            radio.write(message)
            print(message)

            
            #Deactivate Pin
            GPIO.output(pin, GPIO.LOW)
            

        if conf["show_video"]:
            cv2.imshow("Security Feed", frame)
            key = cv2.waitKey(1) & 0xFF

            # Q to break from the loop
            if key == ord("q"):
                break

        rawCapture.truncate(0)
        
GPIO.cleanup()

