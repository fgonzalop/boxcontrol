/*
* BoxDomotic Node to communicate with RPI and others
*/
#include <OneWire.h>
#include <EEPROM.h>
#include <SPI.h>
#include "RF24.h"
#include "printf.h"

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 */
RF24 radio(9,10);

const int RADIO_ID_ADDRESS = 0;

int theRadioNumber = 0;
int aRadioNumber;
int theTemperaturePin;
int thePIRPin;
volatile unsigned long thePIR_START = 0;
int theLuxPin;
byte addresses[][6] = {"BoxDo","BoxDo"};
float theTemperature = 20.0;
void Temperature();
unsigned long currentTime = 0;
byte message[30];
float voltage;
byte theRelay1 = 0;
byte theRelay2 = 0;
byte theRelay3 = 0;
byte theRelay4 = 0;
byte theRelay5 = 0;
byte theRelay6 = 0;

void setup() {
  int aIndex;

  for (aIndex=0;aIndex<30;aIndex++)
  {
    message[aIndex] = 0;
  }
  
  Serial.begin(115200);
  Serial.println(F("***********************"));
  Serial.println(F("BoxDomotic Node 1.0.0. TEST"));
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

  digitalWrite(8, HIGH);
  digitalWrite(7, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(2, HIGH);
  digitalWrite(A1, HIGH);

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
   message[4]=1;
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
    delay(500);
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

void createMessage()
{
  message[0] = theRadioNumber & 0xFF;
  message[1] = (theRadioNumber >> 8) & 0xFF;
  message[2] = (theRadioNumber >> 16) & 0xFF;
  message[3] = (theRadioNumber >> 24) & 0xFF;
  
  message[5]=int(theTemperature);
  message[6]=int((theTemperature-int(theTemperature))*100.0);
  message[7]=int(voltage);
  message[8]=int((voltage-int(voltage))*100.0);
  message[9]=theRelay1;
  message[10]=theRelay2;
  message[11]=theRelay3;
  message[12]=theRelay4;
  message[13]=theRelay5;
  message[14]=theRelay6;
  message[16]=0; //Time for last PIR event, TBD
  message[17]=0;
  message[18]=0;
  message[19]=0;
}
void Update()
{
  int sensorValue = analogRead(theLuxPin);
     // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  voltage = sensorValue * (5.0 / 1023.0);
  // print out the value you read:
  Serial.print(voltage);

  Temperature();
  Serial.println(theTemperature);
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
  Update();
  digitalWrite(8, LOW);
  theRelay1 = 0xFF;
  createMessage();
  radio.write( &message, 21 ); 
  message[4]=0;
  delay(1000);

  Update();
  digitalWrite(8, HIGH);
  theRelay1 = 0x00;
  digitalWrite(7, LOW);
  theRelay2 = 0xFF;
  createMessage();
  radio.write( &message, 21 ); 
  message[4]=0;
  delay(1000);
  
  Update();
  digitalWrite(7, HIGH);
  theRelay2 = 0x00;
  digitalWrite(6, LOW);
  theRelay3 = 0xFF; 
  createMessage();
  radio.write( &message, 21 ); 
  message[4]=0;
  delay(1000);

  Update();
  digitalWrite(6, HIGH);
  theRelay3 = 0x00;
  digitalWrite(5, LOW);
  theRelay4 = 0xFF;
  createMessage();
  radio.write( &message, 21 ); 
  message[4]=0;
  delay(1000);

  Update();
  digitalWrite(5, HIGH);
  theRelay4 = 0x00;
  digitalWrite(2, LOW);
  theRelay5 = 0xFF;
  createMessage();
  radio.write( &message, 21 ); 
  message[4]=0;
  delay(1000);

  Update();
  digitalWrite(2, HIGH);
  theRelay5 = 0x00;
  digitalWrite(A1, LOW);
  theRelay6 = 0xFF;
  createMessage();
  radio.write( &message, 21 ); 
  message[4]=0;
  delay(1000);
  digitalWrite(A1, HIGH);
  theRelay6 = 0x00;
}