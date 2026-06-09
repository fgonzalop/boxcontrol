/*
* BoxDomotic Node to communicate with RPI and others
*/
#include <EEPROM.h>

#include "printf.h"

const int RADIO_ID_ADDRESS = 0;
int theRadioNumber;

void setup() {
  int aRadioNumber;
  
  Serial.begin(115200);
  Serial.println(F("***********************"));
  Serial.println(F("BoxDomotic SETUP"));
  Serial.println(F("***********************"));

  aRadioNumber = EEPROM.read(RADIO_ID_ADDRESS);
  theRadioNumber = aRadioNumber;
  Serial.println(aRadioNumber);
  aRadioNumber = EEPROM.read(RADIO_ID_ADDRESS+1);
  Serial.println(aRadioNumber);
  theRadioNumber = theRadioNumber + aRadioNumber*0x100;
  aRadioNumber = EEPROM.read(RADIO_ID_ADDRESS+2);
  Serial.println(aRadioNumber);
  theRadioNumber = theRadioNumber + aRadioNumber*0x10000;
  aRadioNumber = EEPROM.read(RADIO_ID_ADDRESS+3);
  Serial.println(aRadioNumber);
  theRadioNumber = theRadioNumber + aRadioNumber*0x1000000;

  Serial.print("Radio ID ");
  Serial.println(theRadioNumber);

  theRadioNumber = 3;
  EEPROM.write(RADIO_ID_ADDRESS, theRadioNumber & 0xFF);
  EEPROM.write(RADIO_ID_ADDRESS+1, (theRadioNumber >> 8) & 0xFF);
  EEPROM.write(RADIO_ID_ADDRESS+2, (theRadioNumber >> 16) & 0xFF);
  EEPROM.write(RADIO_ID_ADDRESS+3, (theRadioNumber >> 24) & 0xFF);

}

void loop() 
{     
  Serial.print("Configurado para radio ID ");
  Serial.println(theRadioNumber);

  delay(1000);
  //Serial.println("Otro");

  
} // Loop

