/*
* BoxDomotic Node to communicate with RPI and others
*/

#include <EEPROM.h>
#include <SPI.h>
#include "RF24.h"
#include "BoxDomoticProtocol.h"
#include "Action.h"

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 108 */
RF24 radio(9,10);

int theRadioNumber;
int isRouter = 1;

struct payload_t {
  byte origen;
  byte messageId;
  byte action1;
  byte action2;
  byte spare;
  byte hop1;
  byte hop2;
  byte hop3;
  byte hop4;
  byte hop5;
  byte hop6;
  byte hop7;
};


byte addresses[][6] = {"BoxDo","BoxDo"};

payload_t payload;
payload_t payload_r;
payload_t payload_original;
payload_t theRouting;
int       isWaitingRouting;
unsigned long theTimeForTimeout;
unsigned long theTimeout = TIMEOUT;
int aCounter = 0;

int hops(payload_t aPayload)
{
  int aHops = 0;

  if (aPayload.hop1 != 0)
  {
    aHops++; 
    if (aPayload.hop2 != 0)
    {
       aHops++;
       if (aPayload.hop3 != 0)
       {
          aHops++;
       }
    }
  }
  
  return aHops;
}
void setup() {
  Serial.begin(115200);
  Serial.println(F("BoxDomotic Node 1.0"));

  //EEPROM.write(RADIO_ID_ADDRESS, 4);

  theRadioNumber = EEPROM.read(RADIO_ID_ADDRESS);
  if (theRadioNumber == 0xFF)
  {
    theRadioNumber = 0xFE;
  }
  Serial.print("Radio ID:");
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
}

void loop() {
   if (isWaitingRouting)
   {
      if ((theTimeForTimeout + theTimeout) < millis())
      {
Serial.println("Timeout");
Serial.println(millis());
Serial.println(theTimeForTimeout);  
      isWaitingRouting = 0;    

      theRouting.messageId = theRouting.messageId+1;
      theRouting.hop1 = theRouting.origen;
      theRouting.origen = theRadioNumber;
      theRouting.action1 = TIMEOUT_ANSWER; //Timeout
      theRouting.action2 = TIMEOUT_ANSWER; //Timeout
      theRouting.hop2    = 0;
      theRouting.hop3    = 0;
      theRouting.hop4    = 0;
      theRouting.hop5    = 0;
      theRouting.hop6    = 0;
      theRouting.hop7    = 0;
      
      radio.stopListening();                                        // First, stop listening so we can talk   
      delay (10);
      radio.write( &theRouting, sizeof(payload_t) );              // Send the final one back.
      delay (10);      
      radio.startListening();
      }
   }  
     
   if( radio.available())
   {
      radio.read( &payload_r, sizeof(payload_t) ); 
      
      if (isWaitingRouting)
      {            
         //Wait answer ROUTING                  
         if ((payload_r.hop1 == theRadioNumber ))
         {
Serial.println("Answer");
Serial.println(millis());
            isWaitingRouting = 0;
            payload_r.hop7 = payload_r.hop6;
            payload_r.hop6 = payload_r.hop5;
            payload_r.hop5 = payload_r.hop4;
            payload_r.hop4 = payload_r.hop3;
            payload_r.hop3 = payload_r.hop2;
            payload_r.hop2 = payload_r.origen;
            payload_r.hop1 = theRouting.origen;            
            payload_r.origen = theRadioNumber;

            radio.stopListening();                                        // First, stop listening so we can talk   
            delay (10);
            radio.write( &payload_r, sizeof(payload_t) );              // Send the final one back.
            delay (10);      
            radio.startListening();
         }

   }else
   {
      if ((payload_r.hop1 == theRadioNumber) && ((payload_r.messageId % 2) == 0))
      {  
Serial.println("mio");   
        if (payload_r.hop2 == 0)
        {
Serial.println("Rx");
          // mensaje directo
Serial.print(payload_r.origen);
          radio.stopListening();                                        // First, stop listening so we can talk   
          //delay (10);
          payload_original = payload_r;
          payload_r.messageId = payload_r.messageId+1;
          payload_r.hop1 = payload_r.origen;
          payload_r.origen = theRadioNumber;
          payload_r.action2 = Answer(payload_r.action1, payload_r.action2);
          payload_r.action1 = 0x00; //STATUS OK
          
          radio.write( &payload_r, sizeof(payload_t) );              // Send the final one back.
          delay (10);      
          radio.startListening();                                       // Now, resume listening so we catch the next packets.  
          Perform(payload_original.action1, payload_original.action2);   
Serial.println(F("Sent response "));
        }else
        { 
Serial.println("Routing msg");
          if (isRouter == 1)
          {
            isWaitingRouting = 1;
            theTimeout = (hops(payload_r)-1)*TIMEOUT;
            theRouting = payload_r;
            payload_r.origen = theRadioNumber;
            payload_r.hop1 = payload_r.hop2;
            payload_r.hop2 = payload_r.hop3;
            payload_r.hop3 = payload_r.hop4;
            payload_r.hop4 = payload_r.hop5;
            payload_r.hop5 = payload_r.hop6;
            payload_r.hop6 = payload_r.hop7;
            payload_r.hop7 = 0;
            theTimeForTimeout = millis();
  
            Serial.println("Routing");
            Serial.println(theTimeForTimeout);
            radio.stopListening();                                        // First, stop listening so we can talk   
            delay (10);
            radio.write( &payload_r, sizeof(payload_t) );              // Send the final one back.
            //delay (10);      
            radio.startListening();
            //delay(10);
            
  Serial.println("Waiting..");
             //
          }
        }
      }
    } 
      //delay (10);
   }
   delay (10);

} // Loop
