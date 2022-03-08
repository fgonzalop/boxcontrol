/*
* BoxDomotic Node to communicate with RPI and others
*/
#include <OneWire.h>
#include <EEPROM.h>
#include <SPI.h>
#include "RF24.h"
#include "BoxDomoticProtocol.h"

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 */
RF24 radio(9,10);

int theRadioNumber;
volatile unsigned long thePIR_START = 0;
bool RxWaiting = true;
int theCurrentMessage = 0;
int theMaxMessages;
unsigned long last_time_message;

byte addresses[][6] = {"BoxDo","BoxDo"};

payload_t payload;
payload_t payload_r;
payload_t payload_original;
payload_t theRouting[10];

void setup() {
  int aIndex =0;

  Serial.begin(115200);
  Serial.println(F("****************************"));
  Serial.println(F("BoxDomotic Topology 1.0.0"));
  Serial.println(F("****************************"));

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
  radio.setPALevel(RF24_PA_HIGH);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);

  int theMasterTxPin = EEPROM.read(MASTER_TX_PIN);
  if (theMasterTxPin == 1)
  {
    RxWaiting = false;  
  }
  else
  {
    RxWaiting = true;
    radio.startListening();
  }
  Serial.print("Master Rx:");
  Serial.println(RxWaiting);

  /* Nivel 1 */
  for (aIndex = 1; aIndex <10; aIndex++)
  {
    if (aIndex == theRadioNumber)
    { 
      aIndex++;
    }
    
    theRouting[theCurrentMessage].messageId = 100;
    theRouting[theCurrentMessage].hop1      = aIndex;
    theRouting[theCurrentMessage].origen    = theRadioNumber;
    theRouting[theCurrentMessage].action.action1 = REQUEST_PIR_ACTION; 
    theRouting[theCurrentMessage].action.action2 = SUCCESS_ANSWER; 
    theRouting[theCurrentMessage].hop2    = 0;
    theRouting[theCurrentMessage].hop3    = 0;
    theRouting[theCurrentMessage].hop4    = 0;
    theRouting[theCurrentMessage].hop5    = 0;
    theRouting[theCurrentMessage].hop6    = 0;
    theRouting[theCurrentMessage].hop7    = 0;
    theRouting[theCurrentMessage].hop_reply1 = theRadioNumber;
    theRouting[theCurrentMessage].hop_reply2 = 0;
    theRouting[theCurrentMessage].hop_reply3 = 0;
    theRouting[theCurrentMessage].hop_reply4 = 0;
    theRouting[theCurrentMessage].hop_reply5 = 0;
    theRouting[theCurrentMessage].hop_reply6 = 0;
    theRouting[theCurrentMessage].hop_reply7 = 0;
  
    Serial.print("TX payload:");
    Serial.print(theRouting[theCurrentMessage].messageId);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].origen);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop1);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop2);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop3);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop4);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop5);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop6);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop7);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].action.action1);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].action.action2);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].action.action3);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].action.action4);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop_reply1);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop_reply2);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop_reply3);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop_reply4);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop_reply5);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop_reply6);
    Serial.print(" ");
    Serial.print(theRouting[theCurrentMessage].hop_reply7);
  
    radio.write( &theRouting[theCurrentMessage], sizeof(payload_t) );              // Send the final one back.
    
    Serial.println(" OK");

    delay (500); 
  } 
  /* Nivel 2 */
  for (int route = 6; route <10; route++)
  {
    for (aIndex = 1; aIndex <10; aIndex++)
    {
      if ((aIndex == theRadioNumber) || (aIndex == route))
      { 
        aIndex++;
      }
      
      theRouting[theCurrentMessage].messageId = 102;
      theRouting[theCurrentMessage].hop1      = route;
      theRouting[theCurrentMessage].origen    = theRadioNumber;
      theRouting[theCurrentMessage].action.action1 = REQUEST_PIR_ACTION; 
      theRouting[theCurrentMessage].action.action2 = SUCCESS_ANSWER; 
      theRouting[theCurrentMessage].hop2    = aIndex;
      theRouting[theCurrentMessage].hop3    = 0;
      theRouting[theCurrentMessage].hop4    = 0;
      theRouting[theCurrentMessage].hop5    = 0;
      theRouting[theCurrentMessage].hop6    = 0;
      theRouting[theCurrentMessage].hop7    = 0;
      theRouting[theCurrentMessage].hop_reply1 = theRadioNumber;
      theRouting[theCurrentMessage].hop_reply2 = 0;
      theRouting[theCurrentMessage].hop_reply3 = 0;
      theRouting[theCurrentMessage].hop_reply4 = 0;
      theRouting[theCurrentMessage].hop_reply5 = 0;
      theRouting[theCurrentMessage].hop_reply6 = 0;
      theRouting[theCurrentMessage].hop_reply7 = 0;
    
      Serial.print("TX payload:");
      Serial.print(theRouting[theCurrentMessage].messageId);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].origen);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop1);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop2);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop3);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop4);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop5);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop6);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop7);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].action.action1);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].action.action2);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].action.action3);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].action.action4);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop_reply1);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop_reply2);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop_reply3);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop_reply4);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop_reply5);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop_reply6);
      Serial.print(" ");
      Serial.print(theRouting[theCurrentMessage].hop_reply7);
    
      radio.write( &theRouting[theCurrentMessage], sizeof(payload_t) );              // Send the final one back.
      
      Serial.println(" OK");
  
      delay (500); 
    }
  } 
}

/*
 * PIR_ISR
 */
void PIR_ISR()
{
   thePIR_START = millis();
}

/*
 * Procedure loop
 *    Se lee la parte de RF los datos.
 *    Se procesa y se manda ANSWER y PERFORM (si es diferido)
 */
void loop() 
{
	
} // Loop
