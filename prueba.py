import netifaces

print (netifaces.interfaces())

for interface in netifaces.interfaces():
    #print (netifaces.ifaddresses(interface))
    print (netifaces.ifaddresses(interface))