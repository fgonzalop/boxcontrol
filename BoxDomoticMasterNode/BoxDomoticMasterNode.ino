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
  Serial.println(F("BoxDomotic Master Node 2.0.1"));
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
  radio.setPALevel(RF24_PA_MAX);
  
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

  aIndex = 0;
  theRouting[aIndex].messageId = 100;
  theRouting[aIndex].hop1      = 6;
  theRouting[aIndex].origen    = theRadioNumber;
  theRouting[aIndex].action.action1 = REQUEST_NEIGHBOUR; 
  theRouting[aIndex].action.action2 = SUCCESS_ANSWER; 
  theRouting[aIndex].hop2    = 0;
  theRouting[aIndex].hop3    = 0;
  theRouting[aIndex].hop4    = 0;
  theRouting[aIndex].hop5    = 0;
  theRouting[aIndex].hop6    = 0;
  theRouting[aIndex].hop7    = 0;
  theRouting[aIndex].hop_reply1 = theRadioNumber;
  theRouting[aIndex].hop_reply2 = 0;
  theRouting[aIndex].hop_reply3 = 0;
  theRouting[aIndex].hop_reply4 = 0;
  theRouting[aIndex].hop_reply5 = 0;
  theRouting[aIndex].hop_reply6 = 0;
  theRouting[aIndex].hop_reply7 = 0;

  aIndex = 1;
  theRouting[aIndex].messageId = 102;
  theRouting[aIndex].hop1      = 5;
  theRouting[aIndex].origen    = theRadioNumber;
  theRouting[aIndex].action.action1 = REQUEST_TEMPERATURE_ACTION; 
  theRouting[aIndex].action.action2 = SUCCESS_ANSWER; 
  theRouting[aIndex].hop2    = 0;
  theRouting[aIndex].hop3    = 0;
  theRouting[aIndex].hop4    = 0;
  theRouting[aIndex].hop5    = 0;
  theRouting[aIndex].hop6    = 0;
  theRouting[aIndex].hop7    = 0;
  theRouting[aIndex].hop_reply1 = theRadioNumber;
  theRouting[aIndex].hop_reply2 = 0;
  theRouting[aIndex].hop_reply3 = 0;
  theRouting[aIndex].hop_reply4 = 0;
  theRouting[aIndex].hop_reply5 = 0;
  theRouting[aIndex].hop_reply6 = 0;
  theRouting[aIndex].hop_reply7 = 0;

  aIndex = 2;
  theRouting[aIndex].messageId = 104;
  theRouting[aIndex].hop1      = 5;
  theRouting[aIndex].origen    = theRadioNumber;
  theRouting[aIndex].action.action1 = REQUEST_TEMPERATURE_ACTION; 
  theRouting[aIndex].action.action2 = SUCCESS_ANSWER; 
  theRouting[aIndex].hop2    = 4;
  theRouting[aIndex].hop3    = 0;
  theRouting[aIndex].hop4    = 0;
  theRouting[aIndex].hop5    = 0;
  theRouting[aIndex].hop6    = 0;
  theRouting[aIndex].hop7    = 0;
  theRouting[aIndex].hop_reply1 = theRadioNumber;
  theRouting[aIndex].hop_reply2 = 0;
  theRouting[aIndex].hop_reply3 = 0;
  theRouting[aIndex].hop_reply4 = 0;
  theRouting[aIndex].hop_reply5 = 0;
  theRouting[aIndex].hop_reply6 = 0;
  theRouting[aIndex].hop_reply7 = 0;

  aIndex = 3;
  theRouting[aIndex].messageId = 106;
  theRouting[aIndex].hop1      = 5;
  theRouting[aIndex].origen    = theRadioNumber;
  theRouting[aIndex].action.action1 = REQUEST_TEMPERATURE_ACTION; 
  theRouting[aIndex].action.action2 = SUCCESS_ANSWER; 
  theRouting[aIndex].hop2    = 4;
  theRouting[aIndex].hop3    = 9;
  theRouting[aIndex].hop4    = 0;
  theRouting[aIndex].hop5    = 0;
  theRouting[aIndex].hop6    = 0;
  theRouting[aIndex].hop7    = 0;
  theRouting[aIndex].hop_reply1 = theRadioNumber;
  theRouting[aIndex].hop_reply2 = 0;
  theRouting[aIndex].hop_reply3 = 0;
  theRouting[aIndex].hop_reply4 = 0;
  theRouting[aIndex].hop_reply5 = 0;
  theRouting[aIndex].hop_reply6 = 0;
  theRouting[aIndex].hop_reply7 = 0;

  aIndex = 4;
  theRouting[aIndex].messageId = 108;
  theRouting[aIndex].hop1      = 5;
  theRouting[aIndex].origen    = theRadioNumber;
  theRouting[aIndex].action.action1 = REQUEST_TEMPERATURE_ACTION; 
  theRouting[aIndex].action.action2 = SUCCESS_ANSWER; 
  theRouting[aIndex].hop2    = 4;
  theRouting[aIndex].hop3    = 9;
  theRouting[aIndex].hop4    = 3;
  theRouting[aIndex].hop5    = 0;
  theRouting[aIndex].hop6    = 0;
  theRouting[aIndex].hop7    = 0;
  theRouting[aIndex].hop_reply1 = theRadioNumber;
  theRouting[aIndex].hop_reply2 = 0;
  theRouting[aIndex].hop_reply3 = 0;
  theRouting[aIndex].hop_reply4 = 0;
  theRouting[aIndex].hop_reply5 = 0;
  theRouting[aIndex].hop_reply6 = 0;
  theRouting[aIndex].hop_reply7 = 0;

  aIndex = 5;
  theRouting[aIndex].messageId = 110;
  theRouting[aIndex].hop1      = 6;
  theRouting[aIndex].origen    = theRadioNumber;
  theRouting[aIndex].action.action1 = REQUEST_TEMPERATURE_ACTION; 
  theRouting[aIndex].action.action2 = SUCCESS_ANSWER; 
  theRouting[aIndex].hop2    = 5;
  theRouting[aIndex].hop3    = 4;
  theRouting[aIndex].hop4    = 2;
  theRouting[aIndex].hop5    = 0;
  theRouting[aIndex].hop6    = 0;
  theRouting[aIndex].hop7    = 0;
  theRouting[aIndex].hop_reply1 = theRadioNumber;
  theRouting[aIndex].hop_reply2 = 0;
  theRouting[aIndex].hop_reply3 = 0;
  theRouting[aIndex].hop_reply4 = 0;
  theRouting[aIndex].hop_reply5 = 0;
  theRouting[aIndex].hop_reply6 = 0;
  theRouting[aIndex].hop_reply7 = 0;
  theMaxMessages = 5; 
  
  last_time_message = millis();
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
	if (RxWaiting)
	{
		if( radio.available())
		{
		  payload_r.messageId=1; //Only to enter once
		  radio.read( &payload_r, sizeof(payload_t) ); 
			
      Serial.print("RX payload:");
      Serial.print(payload_r.messageId);
      Serial.print(" ");
      Serial.print(payload_r.origen);
      Serial.print(" ");
      Serial.print(payload_r.hop1);
      Serial.print(" ");
      Serial.print(payload_r.hop2);
      Serial.print(" ");
      Serial.print(payload_r.hop3);
      Serial.print(" ");
      Serial.print(payload_r.hop4);
      Serial.print(" ");
      Serial.print(payload_r.hop5);
      Serial.print(" ");
      Serial.print(payload_r.hop6);
      Serial.print(" ");
      Serial.print(payload_r.hop7);
      Serial.print(" ");
      Serial.print(payload_r.action.action1);
      Serial.print(" ");
      Serial.print(payload_r.action.action2);
      Serial.print(" ");
      Serial.print(payload_r.action.action3);
      Serial.print(" ");
      Serial.print(payload_r.action.action4);
      Serial.print("(+");
      Serial.print(millis()-last_time_message);
      last_time_message = millis();
      Serial.print(")");
      Serial.print(" ");
      Serial.print(payload_r.hop_reply1);
      Serial.print(" ");
      Serial.print(payload_r.hop_reply2);
      Serial.print(" ");
      Serial.print(payload_r.hop_reply3);
      Serial.print(" ");
      Serial.print(payload_r.hop_reply4);
      Serial.print(" ");
      Serial.print(payload_r.hop_reply5);
      Serial.print(" ");
      Serial.print(payload_r.hop_reply6);
      Serial.print(" ");
      Serial.println(payload_r.hop_reply7);
		}
   else
   {
    //Serial.print(".");

   }
	}
	else
	{
    Serial.print("TX ...");
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

   if (theCurrentMessage == theMaxMessages) //Máximo número de mensajes en la lista.
   {
    theCurrentMessage = 0;
   }
   else
   {
    theCurrentMessage++;
   }
   delay (3000);
	}

} // Loop
