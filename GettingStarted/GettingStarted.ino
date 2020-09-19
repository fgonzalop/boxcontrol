
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
int theRadioNumber = 2;
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

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(9,10);
/**********************************************************/

byte addresses[][6] = {"BoxDo","BoxDo"};

payload_t payload;
payload_t payload_r;
payload_t theRouting;
int       isWaitingRouting;
int aCounter = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("BoxDomotic Node"));
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
   if( radio.available())
   {
    if (isWaitingRouting)
    {      
         //Wait answer ROUTING
         radio.read( &payload_r, sizeof(payload_t) );      
                  
         if ((payload_r.hop1 == theRadioNumber ))
         {
          Serial.println("Answer");
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
      //while (radio.available()) 
      //{                                   // While there is data ready
        radio.read( &payload_r, sizeof(payload_t) );             // Get the payload
      //}
      if (payload_r.hop1 == theRadioNumber)
      {  
        Serial.println("mio");   
        if (payload_r.hop2 == 0)
        {
          Serial.println("Rx");
          // mensaje directo
          Serial.print(payload_r.origen);
          radio.stopListening();                                        // First, stop listening so we can talk   
          //delay (10);
          payload_r.messageId = payload_r.messageId+1;
          payload_r.hop1 = payload_r.origen;
          payload_r.origen = theRadioNumber;
          
          radio.write( &payload_r, sizeof(payload_t) );              // Send the final one back.
          delay (10);      
          radio.startListening();                                       // Now, resume listening so we catch the next packets.     
          Serial.println(F("Sent response "));
        }else
        { 
          Serial.println("Routing msg");
          if (isRouter == 1)
          {
            isWaitingRouting = 1;
            theRouting = payload_r;
            payload_r.origen = theRadioNumber;
            payload_r.hop1 = payload_r.hop2;
            payload_r.hop2 = payload_r.hop3;
            payload_r.hop3 = payload_r.hop4;
            payload_r.hop4 = payload_r.hop5;
            payload_r.hop5 = payload_r.hop6;
            payload_r.hop6 = payload_r.hop7;
            payload_r.hop7 = 0;
  
            Serial.println("Routing");
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
