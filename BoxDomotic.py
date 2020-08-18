#!/usr/bin/env python3

import paho.mqtt.client as mqtt

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
topic = [['data/RF24/Node0Lux', 'data/RF24/Node0Tra'], ['data/RF24/Node1Lux', 'data/RF24/Node1Tra']]
#data = 24

# radio setup for RPi B Rev2: CS0=Pin 25
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
            print("{}".format(topic[mesh.getNodeID(header.from_node)][header.type]))
            mqttc.publish(topic[mesh.getNodeID(header.from_node)][header.type], data)
            mqttc.loop(2)
            
    mqttc.loop_stop()
