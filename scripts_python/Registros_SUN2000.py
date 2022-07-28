from pymodbus.client.sync import ModbusTcpClient as ModbusClient
import time

import paho.mqtt.client as mqtt
import ctypes

#your inverter ip
ip_inverter='192.168.100.42'
broker_address = '192.168.100.199'
clientmqtt = mqtt.Client('sun2000-inverter') # Creación del cliente
clientmqtt.connect(broker_address)

topic = "inverter/model"
clientmqtt.publish(topic, "SUN2000L-5KTL")

topic = "inverter/ip"
clientmqtt.publish(topic, ip_inverter);

topic = "inverter/modbus_tcp/port"
clientmqtt.publish(topic, "502");
topic = "inverter/modbus_tcp/unit_id"
clientmqtt.publish(topic, "1");

client = ModbusClient(ip_inverter, port=502)
client.connect()
time.sleep(1)
if client.connect():
    # The trick is use  unit_id=1 not in ModbusClient() but in client.read_holding_registers()
    request = client.read_holding_registers(address=30070,count= 1, unit=0)
    print (request.registers[0]) #300
    #print (request.registers[1])
else:
    print ('something is wrong with IP or port')

    
request = client.read_holding_registers(address=30071,count= 1, unit=0)
print (request.registers[0])

topic = "inverter/pv_string"
clientmqtt.publish(topic, request.registers[0]);

request = client.read_holding_registers(address=32016,count= 1, unit=0)
print (request.registers[0])
topic = "inverter/pv_1/voltage"
clientmqtt.publish(topic, request.registers[0]);

request = client.read_holding_registers(address=32017,count= 1, unit=0)
print (request.registers[0])
topic = "inverter/pv_1/current"
clientmqtt.publish(topic, request.registers[0]);

request = client.read_holding_registers(address=32018,count= 1, unit=0)
print (request.registers[0])
topic = "inverter/pv_2/voltage"
clientmqtt.publish(topic, request.registers[0]);

request = client.read_holding_registers(address=32019,count= 1, unit=0)
print (request.registers[0])	
topic = "inverter/pv_2/current"
clientmqtt.publish(topic, request.registers[0]);

request = client.read_holding_registers(address=32000,count= 1, unit=0)
print (request.registers[0])
value="--"	
if (request.registers[0] == 0) :
    value="TBD"
if (request.registers[0] == 1) :
    value="standby"  

topic = "inverter/status"
clientmqtt.publish(topic, value);

#request = client.read_holding_registers(address=32085,count= 1, unit=0)
#print (request.registers[0])

print("Grid current")
request = client.read_holding_registers(address=32072,count= 2, unit=0)
print (request.registers[0])
print (request.registers[1])
topic = "inverter/solar_current"
clientmqtt.publish(topic, float(request.registers[0]*0x10000+request.registers[1])/1000.0 );

print("Grid METER current")
request = client.read_holding_registers(address=37107,count= 2, unit=0)
print (request.registers[0])
print (request.registers[1])
print (int(0x10000*request.registers[0]+request.registers[1]), type(request.registers[0]))
m=int(0x10000*request.registers[0]+request.registers[1]+1)
print (ctypes.c_int32(~m).value)
topic = "inverter/grid/current"
clientmqtt.publish(topic, ctypes.c_int32(~m).value);

#request = client.read_holding_registers(address=32089,count= 1, unit=0)
#print (request.registers[0])

request = client.read_holding_registers(address=37113,count= 2, unit=0)
print (request.registers[0])
print (request.registers[1])
m=int(0x10000*request.registers[0]+request.registers[1]+1)
print (ctypes.c_int32(~m).value)
topic = "inverter/grid/power"
clientmqtt.publish(topic, ctypes.c_int32(~m).value);

request = client.read_holding_registers(address=37101,count= 2, unit=0)
print (request.registers[0])
print (request.registers[1])
topic = "inverter/grid/voltage"
clientmqtt.publish(topic, request.registers[1]/10.0)

print("input power")
request = client.read_holding_registers(address=32064,count= 2, unit=0)
print (request.registers[0])
print (request.registers[1])
topic = "inverter/solar/power"
clientmqtt.publish(topic, request.registers[1]);

print("Grid voltage")
request = client.read_holding_registers(address=32066,count= 1, unit=0)
print (request.registers[0])
topic = "inverter/solar/voltage"
clientmqtt.publish(topic, request.registers[0]/10.0)


