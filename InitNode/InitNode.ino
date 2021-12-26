/*
* Init Node of BoxDomotic  to communicate with RPI and others
*/
#include <OneWire.h>
#include <EEPROM.h>
#include <SPI.h>
#include "RF24.h"
#include "Cochera.h"
#include "BoxDomoticProtocol.h"

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 */
RF24 radio(9,10);
int theRadioNumber;
int isRouter = 1;
int theRelayIndex;
int theTemperaturePin;
int thePIRPin;
volatile int thePIR = 0;
int theLuxPin;

byte addresses[][6] = {"BoxDo","BoxDo"};

unsigned long theTemperature = 20;
int theRelay[MAX_RELAY] = {0,0,0,0,0,0,0,0,0,0};

int       isWaitingRouting;

void setup() {
  int aIndex;
  
  Serial.begin(115200);
  Serial.println(F("INIT BoxDomotic Node 1.0"));
  Serial.println(THE_NAME);
  
  EEPROM.write(RADIO_ID_ADDRESS, THE_RADIO_ID);
  EEPROM.write(RELAY_INDEX, THE_RELAY_INDEX); //1 relay operativo
  EEPROM.write(RELAY_INDEX+1, THE_RELAY_0);
  EEPROM.write(RELAY_INDEX+2, THE_RELAY_1);
  EEPROM.write(RELAY_INDEX+3, THE_RELAY_2);
  EEPROM.write(RELAY_INDEX+4, THE_RELAY_3);
  EEPROM.write(RELAY_INDEX+5, THE_RELAY_5);
  EEPROM.write(RELAY_INDEX+6, THE_RELAY_6);
  EEPROM.write(RELAY_INDEX+7, THE_RELAY_7);
  EEPROM.write(RELAY_INDEX+8, THE_RELAY_8);
  EEPROM.write(RELAY_INDEX+9, THE_RELAY_9);
  
  EEPROM.write(TEMPERATURE_PIN, THE_TEMPERATURE_PIN);
  EEPROM.write(PIR_PIN, THE_PIR_PIN);
  EEPROM.write(LUX_PIN, THE_LUX_PIN);
  
  // Read and show  
  Serial.println("ID ADDRESS:");
  for (int aIndex = 0; aIndex<20; aIndex++)
  {
     Serial.println(EEPROM.read(RADIO_ID_ADDRESS+aIndex));
  }

  // Configure and check
  theRadioNumber = EEPROM.read(RADIO_ID_ADDRESS);
  if (theRadioNumber == 0xFF)
  {
    theRadioNumber = 0xFE;
  }
  Serial.print("Radio ID ");
  Serial.println(theRadioNumber);
  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
    
  // Start the radio listening for data
  radio.startListening();

  isWaitingRouting = 0;

  theRelayIndex = EEPROM.read(RELAY_INDEX);
  if (theRelayIndex == 0xFF)
  {
Serial.println("Relay's not configured");
     for (aIndex=0; aIndex<MAX_RELAY; aIndex++)
     {
        theRelay[aIndex]= 0;
     }
      
  }else
  {
Serial.print("Relay index ");
Serial.println(theRelayIndex);
     for (aIndex=0; aIndex<theRelayIndex; aIndex++)
     {
        theRelay[aIndex]= EEPROM.read(RELAY_INDEX+aIndex+1);
        pinMode(theRelay[aIndex], OUTPUT);
        digitalWrite(theRelay[aIndex], LOW);
     }   
     
     delay (1500);
     for (aIndex=0; aIndex<theRelayIndex; aIndex++)
     {
        digitalWrite(theRelay[aIndex], HIGH);
     }

   }
//EEPROM.write(TEMPERATURE_PIN, 4);// pin 4 Temperatura

   theTemperaturePin = EEPROM.read(TEMPERATURE_PIN);
   if (theTemperaturePin == 0xFF)
   {
Serial.println("Temperature PIN not configured");      
   }
   else
   {
Serial.print("Configuring Temperature pin ");
Serial.print(theTemperaturePin); 
Serial.print(" ");  
//TBD Temperature();
Serial.println(" OK");
  }
  
  //EEPROM.write(PIR_PIN, 3);// pin 3 PIR
  thePIRPin = EEPROM.read(PIR_PIN);

  if (thePIRPin == 0xFF)
  {
    Serial.println("PIR not configured");  
  }
  else
  {
     Serial.print("Configuring PIR pin ");
     Serial.print(thePIRPin); 
     pinMode(thePIRPin, INPUT_PULLUP);
//TBD     attachInterrupt(digitalPinToInterrupt(thePIRPin), PIR_ISR, RISING);
     
     Serial.println(" OK");
  }
  
  //EEPROM.write(LUX_PIN, 7);// pin 7 LUX
  theLuxPin = EEPROM.read(LUX_PIN);

  if (theLuxPin == 0xFF)
  {
    Serial.println("LUX not configured");  
  }
  else
  {
     Serial.print("Configuring LUX pin ");
     Serial.print(theLuxPin); 
     Serial.print(" ");
     int sensorValue = analogRead(A7);//TBD
     // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
     float voltage = sensorValue * (5.0 / 1023.0);
     // print out the value you read:
     Serial.print(voltage);
     Serial.println(" OK");
  }
}


/*
 * Procedure loop
 *    Se lee la parte de RF los datos.
 *    Se procesa y se manda ANSWER y PERFORM (si es diferido)
 */
void loop() 
{
   
} // Loop
