#!/usr/bin/env python3

import paho.mqtt.client as mqtt
import json 

from RF24 import *
from RF24Network import *
from RF24Mesh import *

from struct import unpack

# Opening JSON file with the information of the network
with open('BoxDomotic.json') as json_file: 
    aJsonNetwork = json.load(json_file)

def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    print("message topic=",message.topic)
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)
                
    #mesh.write(aTemperature, octlit("2"), sizeof(aTemperature))
    print("end...")
    
    
mqttHost = "127.0.0.1"
    
#configuration of RPi
radio = RF24(25,0)
network = RF24Network(radio)
mesh = RF24Mesh(radio, network)

mesh.setNodeID(0)
mesh.begin()
radio.setPALevel(RF24_PA_MAX) # Power Amplifier
#radio.printDetails()

print("Starting BoxDomotic...")

print("Cleaning mesh")


mqttc = mqtt.Client()
mqttc.on_message = on_message;
mqttc.connect(mqttHost, 1883)

mqttc.subscribe("calefaccion")
mqttc.subscribe("cocina/tra/ask")

while 1:
    mesh.DHCP()
    mesh.update()
    mqttc.loop_start()

    while network.available():
        header, payload = network.read(10)
        if chr(header.type) == 'M':
            print("Rcv {} from 0{:o}".format(unpack("L",payload)[0], header.from_node))
        else:
            print("Rcv bad type {} from 0{:o}".format(header.type,header.from_node));
            print("{}".format(mesh.getNodeID(header.from_node)))
            

            data = unpack("L",payload)[0]

            print("{}".format(aJsonNetwork[str(mesh.getNodeID(header.from_node))][0]['nombre']))
            mqttc.publish(aJsonNetwork[str(mesh.getNodeID(header.from_node))][0]['nombre'], data)

            mqttc.loop(1)
            
            
            if not network.write(header, payload):
                # If a write fails, check connectivity to the mesh network
                print("failed")
            else:
                print("Send OK")
                
    mqttc.loop_stop()
