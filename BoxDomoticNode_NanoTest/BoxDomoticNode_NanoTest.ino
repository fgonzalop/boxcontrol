/*
* BoxDomotic Node to communicate with RPI and others
*/
#include <OneWire.h>
#include <EEPROM.h>
#include <SPI.h>
#include "RF24.h"
#include "BoxDomoticProtocol.h"
#include "printf.h"

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 */
RF24 radio(9,10);

//const int RADIO_ID_ADDRESS = 0;

int theRadioNumber = 0;
int aRadioNumber;
int theRelayIndex;
int theTemperaturePin;
int thePIRPin;
volatile unsigned long thePIR_START = 0;
int theLuxPin;
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
void Temperature();

unsigned long currentTime = 0;
byte message[30];
float voltage;

void setup() {
  int aIndex;

  for (aIndex=0;aIndex<30;aIndex++)
  {
    message[aIndex] = 0;
  }
  
  Serial.begin(115200);
  Serial.println(F("***********************"));
  Serial.println(F("BoxDomotic Node 2.0.4. TEST"));
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

  message[0] = theRadioNumber & 0xFF;
  message[1] = (theRadioNumber >> 8) & 0xFF;
  message[2] = (theRadioNumber >> 16) & 0xFF;
  message[3] = (theRadioNumber >> 24) & 0xFF;
  
  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_HIGH);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
  radio.setRetries(0, 15);  // ARD=5, ARC=15
    
  // Start the radio listening for data
  //radio.startListening();
  printf_begin();
  radio.printDetails();
  
  //Pins for relays
  pinMode(8, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(A1, OUTPUT);

  digitalWrite(8, LOW);
  digitalWrite(7, LOW);
  digitalWrite(6, LOW);
  digitalWrite(5, LOW);
  digitalWrite(2, LOW);
  digitalWrite(A1, LOW);

  theTemperaturePin = EEPROM.read(TEMPERATURE_PIN);
  theTemperaturePin=4;
  Serial.print(" ");  
  Temperature();
  Serial.println(theTemperature);
  Serial.println(" OK");
  
  thePIRPin = 3;
  Serial.print("Configuring PIR pin ");
  Serial.print(thePIRPin); 
  pinMode(thePIRPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(thePIRPin), PIR_ISR, RISING);   
  Serial.println(" OK");
  
  Serial.print("Configuring LUX pin ");
  theLuxPin = A0;
  int sensorValue = analogRead(theLuxPin);
     // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  voltage = sensorValue * (5.0 / 1023.0);
  // print out the value you read:
  Serial.print(voltage);
  Serial.println(" OK");

  radio.stopListening();
  
}

/*
 * PIR_ISR
 */
void PIR_ISR()
{
   thePIR_START = millis();
}


void Temperature()
{
  OneWire ds(theTemperaturePin);
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;
  
  if (!ds.search(addr))
  {
    theTemperature = 25.0;//TBD
    return;
  }
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);
  
  delay(1000);
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);
  
  for ( i = 0; i < 9; i++) 
  {           // we need 9 bytes
    data[i] = ds.read();
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
  //Serial.println();
  
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  
  celsius = (float)raw / 16.0;
  
  theTemperature = celsius;
  Serial.println(celsius);
}

void loop() 
{     
  
  if (thePIR_START != currentTime)
  {
    Serial.print("PIR... ");
    currentTime = thePIR_START;
    message[4]=1;
  }

  delay(1000);
  //Serial.println("Otro");
  Temperature();
  message[5]=int(theTemperature);
  message[6]=int((theTemperature-int(theTemperature))*100.0);
  message[7]=int(voltage);
  message[8]=int((voltage-int(voltage))*100.0);
  radio.write( &message, 21 );             
  message[4]=0;
}

/*
      if (payload_r.hop1 == theRadioNumber)
      {    
        if (payload_r.hop2 == 0)  // mensaje directo
        {
          payload_original = payload_r;
          payload_r.messageId = payload_r.messageId+1;
          
          payload_r.origen = theRadioNumber;
          payload_r.action = Answer(payload_r.action);
          payload_r.hop1 = payload_r.hop_reply1;
          payload_r.hop2 = payload_r.hop_reply2;
          payload_r.hop3 = payload_r.hop_reply3;
          payload_r.hop4 = payload_r.hop_reply4;
          payload_r.hop5 = payload_r.hop_reply5;
          payload_r.hop6 = payload_r.hop_reply6;
          payload_r.hop7 = payload_r.hop_reply7;

          radio.stopListening();                                        // First, stop listening so we can talk   
          delay (10);
          radio.write( &payload_r, sizeof(payload_t) );              // Send the final one back.
          delay (5);      
          radio.startListening();                                       // Now, resume listening so we catch the next packets.  
Serial.print("requesting..." );
      Perform(payload_original.action);   
Serial.println(F("Sent response "));
        }
		else
        { 
Serial.println("Routing msg");
          payload_r.origen = theRadioNumber;
          payload_r.hop1 = payload_r.hop2;
          payload_r.hop2 = payload_r.hop3;
          payload_r.hop3 = payload_r.hop4;
          payload_r.hop4 = payload_r.hop5;
          payload_r.hop5 = payload_r.hop6;
          payload_r.hop6 = payload_r.hop7;
          payload_r.hop7 = 0;
          payload_r.hop_reply7 = payload_r.hop_reply6;
          payload_r.hop_reply6 = payload_r.hop_reply5;
          payload_r.hop_reply5 = payload_r.hop_reply4;
          payload_r.hop_reply4 = payload_r.hop_reply3;
          payload_r.hop_reply3 = payload_r.hop_reply2;
          payload_r.hop_reply2 = payload_r.hop_reply1;
          payload_r.hop_reply1 = theRadioNumber;
          
          radio.stopListening();                                        // First, stop listening so we can talk   
          delay (10);
          radio.write( &payload_r, sizeof(payload_t) );              // Send the final one back.
          delay (5);      
          radio.startListening();
          //delay(10);
        }
      }
    } 
   delay (1);
   //Serial.print(thePIR);

} // Loop
*/
