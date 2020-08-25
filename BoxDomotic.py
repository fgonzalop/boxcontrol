#!/usr/bin/env python3

import paho.mqtt.client as mqtt
import json 

from RF24 import *
from RF24Network import *
from RF24Mesh import *

from struct import unpack

def on_message(client, userdata, message):
    print("message received " ,str(message.payload.decode("utf-8")))
    print("message topic=",message.topic)
    print("message qos=",message.qos)
    print("message retain flag=",message.retain)
    
    
mqttHost = "127.0.0.1"

# Opening JSON file with the information of the network
with open('BoxDomotic.json') as json_file: 
    aJsonNetwork = json.load(json_file)
    
topic = [['data/RF24/Node0Lux', 'data/RF24/Node0Tra'], ['data/RF24/Node1Lux', 'data/RF24/Node1Tra']]
#data = 24

# radio setup for RPi B Rev2: CS0=Pin 24
#radio = RF24(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ)
radio = RF24(25,0)
network = RF24Network(radio)
mesh = RF24Mesh(radio, network)

mesh.setNodeID(0)
mesh.begin()
radio.setPALevel(RF24_PA_MAX) # Power Amplifier
radio.printDetails()

print("Starting BoxDomotic...")

mqttc = mqtt.Client()
mqttc.on_message = on_message;
mqttc.connect(mqttHost, 1883)
mqttc.subscribe("calefaccion")

while 1:
    mesh.update()
    mesh.DHCP()
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
            #mqttc.publish(topic[mesh.getNodeID(header.from_node)][header.type], data)
            mqttc.loop(2)
            
    mqttc.loop_stop()
