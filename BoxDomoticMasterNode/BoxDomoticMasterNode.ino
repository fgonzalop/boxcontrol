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
int isRouter = 1;
int theRelayIndex;
int theTemperaturePin;
int thePIRPin;
volatile unsigned long thePIR_START = 0;
int theLuxPin;
bool RxWaiting = true;
int theCurrentMessage = 0;

byte addresses[][6] = {"BoxDo","BoxDo"};

payload_t payload;
payload_t payload_r;
payload_t payload_original;
payload_t theRouting[10];
int       isWaitingRouting;
unsigned long theTimeForTimeout;
unsigned long theTimeout = TIMEOUT;
int aCounter = 0;
answer_t aAnswer;

float theTemperature = 20.0;
int theRelay[MAX_RELAY] = {0,0,0,0,0,0,0,0,0,0};

void setup() {
  int aIndex =0;

  Serial.begin(115200);
  Serial.println(F("****************************"));
  Serial.println(F("BoxDomotic Master Node 1.0.2"));
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
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
    
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
Serial.println(" OK");
  }
  
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
     attachInterrupt(digitalPinToInterrupt(thePIRPin), PIR_ISR, RISING);
     
     Serial.println(" OK");
  }
  
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
     theLuxPin = A7; //TBD
     int sensorValue = analogRead(theLuxPin);
     // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
     float voltage = sensorValue * (5.0 / 1023.0);
     // print out the value you read:
     Serial.print(voltage);
     Serial.println(" OK");
  }

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
  theRouting[aIndex].action.action1 = REQUEST_TEMPERATURE_ACTION; 
  theRouting[aIndex].action.action2 = SUCCESS_ANSWER; 
  theRouting[aIndex].hop2    = 0;
  theRouting[aIndex].hop3    = 0;
  theRouting[aIndex].hop4    = 0;
  theRouting[aIndex].hop5    = 0;
  theRouting[aIndex].hop6    = 0;
  theRouting[aIndex].hop7    = 0;

  aIndex = 1;
  theRouting[aIndex].messageId = 100;
  theRouting[aIndex].hop1      = 6;
  theRouting[aIndex].origen    = theRadioNumber;
  theRouting[aIndex].action.action1 = REQUEST_TEMPERATURE_ACTION; 
  theRouting[aIndex].action.action2 = SUCCESS_ANSWER; 
  theRouting[aIndex].hop2    = 5;
  theRouting[aIndex].hop3    = 0;
  theRouting[aIndex].hop4    = 0;
  theRouting[aIndex].hop5    = 0;
  theRouting[aIndex].hop6    = 0;
  theRouting[aIndex].hop7    = 0;
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
Serial.println(payload_r.action.action4);
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
	  radio.write( &theRouting[theCurrentMessage], sizeof(payload_t) );              // Send the final one back.
	  Serial.println(" OK");

   if (theCurrentMessage == 1) //Máximo número de mensajes en la lista.
   {
    theCurrentMessage = 0;
   }
   else
   {
    theCurrentMessage++;
   }
   delay (2000);
	}

} // Loop
