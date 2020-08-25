#!/usr/bin/env python3
# importing the module 
import json 
  
# Opening JSON file 
with open('BoxDomotic.json') as json_file: 
    data = json.load(json_file) 
  
    # Print the type of data variable 
    print("Type:", type(data)) 
  
    # Print the data of dictionary 
    print("\nPeople1:", data['1']) 
    print("\nPeople2:", data['2'])

    print("camino ", data['1'][0]['nombre'])
