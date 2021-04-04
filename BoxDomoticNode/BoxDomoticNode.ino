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
volatile int thePIR = 0;

byte addresses[][6] = {"BoxDo","BoxDo"};

payload_t payload;
payload_t payload_r;
payload_t payload_original;
payload_t theRouting;
int       isWaitingRouting;
unsigned long theTimeForTimeout;
unsigned long theTimeout = TIMEOUT;
int aCounter = 0;
answer_t aAnswer;

unsigned long theTemperature = 20;
int theRelay[MAX_RELAY] = {0,0,0,0,0,0,0,0,0,0};

void setup() {
  int aIndex;
  
  Serial.begin(115200);
  Serial.println(F("BoxDomotic Node 1.0.c"));

  /*
   * EEPROM.write(RADIO_ID_ADDRESS, 2);
   * EEPROM.write(RELAY_INDEX, 255); //1 relay operativo
  EEPROM.write(RELAY_INDEX+1, 255);// pin 2 de relay (0)
  EEPROM.write(RELAY_INDEX+2, 255);// pin 2 de relay (0)
  EEPROM.write(RELAY_INDEX+3, 255);// pin 2 de relay (0)
  EEPROM.write(RELAY_INDEX+4, 255);// pin 2 de relay (0)
  EEPROM.write(RELAY_INDEX+5, 255);// pin 2 de relay (0)
  EEPROM.write(RELAY_INDEX+6, 255);// pin 2 de relay (0)
  EEPROM.write(RELAY_INDEX+7, 255);// pin 2 de relay (0)
  EEPROM.write(RELAY_INDEX+8, 255);// pin 2 de relay (0)
  EEPROM.write(RELAY_INDEX+9, 255);// pin 2 de relay (0)
  */
  
  //
  /*
  Serial.println("ID ADDRESS:");
  Serial.println(EEPROM.read(RADIO_ID_ADDRESS));
  Serial.println(EEPROM.read(RELAY_INDEX));
  Serial.println(EEPROM.read(RELAY_INDEX+1));
  Serial.println(EEPROM.read(RELAY_INDEX+2));
  Serial.println(EEPROM.read(RELAY_INDEX+3));
  Serial.println(EEPROM.read(RELAY_INDEX+4));
  Serial.println(EEPROM.read(RELAY_INDEX+5));
  Serial.println(EEPROM.read(RELAY_INDEX+6));
  Serial.println(EEPROM.read(RELAY_INDEX+7));
  Serial.println(EEPROM.read(RELAY_INDEX+8));
  Serial.println(EEPROM.read(RELAY_INDEX+9));
  Serial.println(EEPROM.read(RELAY_INDEX+10));
  */
  //

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
Temperature();
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
     attachInterrupt(digitalPinToInterrupt(thePIRPin), PIR_ISR, RISING);
     
     Serial.println(" OK");
  }
  
}

/*
 * PIR_ISR 
 */
void PIR_ISR()
{
    thePIR++;  
}

/*
 * Procedure loop
 *    Se lee la parte de RF los datos.
 *    Se procesa y se manda ANSWER y PERFORM (si es diferido)
 */
void loop() 
{
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
      theRouting.action.action1 = TIMEOUT_ANSWER; //Timeout
      theRouting.action.action2 = TIMEOUT_ANSWER; //Timeout
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
      payload_r.messageId=1; //Only to enter once
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
          payload_r.action = Answer(payload_r.action);
          
          radio.write( &payload_r, sizeof(payload_t) );              // Send the final one back.
          delay (10);      
          radio.startListening();                                       // Now, resume listening so we catch the next packets.  
          Perform(payload_r.action);   
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
   //Serial.print(thePIR);

} // Loop

/*
 * Procedure ANSWER
 *   Dependiendo de la ACCION se hará una llamada a una función definida
 *   
 */
answer_t Answer(answer_t aAction)
{
  answer_t aResult;
  switch (aAction.action1)
  {
    case REQUEST_TEMPERATURE_ACTION:
       aResult.action1 = SUCCESS_ANSWER;
       aResult.action2 = AnswerTemperature(aAction);
Serial.print("Temperature (");
Serial.print(aAction.action2);
Serial.print(")");
Serial.println(aResult.action2);
       break;
    case REQUEST_LUX_ACTION:
       aResult.action1 = SUCCESS_ANSWER;
       aResult.action2 = AnswerLux(aAction);
Serial.print("Lux (");
Serial.print(aAction.action2);
Serial.print(")");
Serial.println(aResult.action2);
       break;
    case REQUEST_ON_RELAY_ACTION:
       aResult.action1 = SUCCESS_ANSWER;
       if ( aAction.action2 > MAX_RELAY-1)
       {  //Out of range
          aResult.action2 = NO_ANSWER;
          Serial.println("Value out of range");
          break;
       }
       
       if (theRelay[aAction.action2] == 0)
       {  //No relay conected in this value
          aResult.action2 = NO_ANSWER;
          Serial.println("No relay pin connected");
       }else
       {
          aResult.action2 = SUCCESS_ANSWER; 
          digitalWrite(theRelay[aAction.action2], HIGH);
       }
       
Serial.print("ON_RELAY (");
Serial.print(aAction.action2);
Serial.print(")");
Serial.println(aResult.action2);
       break;
    case REQUEST_OFF_RELAY_ACTION:
       aResult.action1 = SUCCESS_ANSWER;
       if ( aAction.action2 > MAX_RELAY-1)
       {  //Out of range
          aResult.action2 = NO_ANSWER;
          Serial.println("Value out of range");
          break;
       }
       
       if (theRelay[aAction.action2] == 0)
       {  //No relay conected in this value
          aResult.action2 = NO_ANSWER;
          Serial.println("No relay pin connected");
       }else
       {
          aResult.action2 = SUCCESS_ANSWER; 
          digitalWrite(theRelay[aAction.action2], LOW);
       }
       
Serial.print("OFF_RELAY (");
Serial.print(aAction.action2);
Serial.print(")");
Serial.println(aResult.action2);
       break;   
    default:
       aResult.action1 = NO_ANSWER;
       aResult.action2 = 0;
Serial.print("DEFAULT ");
Serial.println(aResult.action2);
       break;
  }

  return aResult;
}

/*
 * Procedure PERFORM
 *    Sólo se ejecutará algo diferido si el tiempo de consulta es alto, p.e. TEMPERATURA
 *    
 */

void Perform(answer_t aAction)
{
  switch (aAction.action1)
  {
    case REQUEST_TEMPERATURE_ACTION:
       PerformTemperature(aAction);
       break;
    default:
       break;
  }
}

/*
 * Procedure ANSWER (TEMPERATURE)
 *    Aquí se devuelve la temperatura última leída.
 */
unsigned long AnswerTemperature(answer_t aAction)
{
   return theTemperature;
}

/*
 * Procedure PERFORM (TEMPERATURE)
 *    Lectura de la temperatura en modo DS18B20
 *    
 */
void PerformTemperature(answer_t aAction)
{
  //OneWire  ds(aAction.action2);
  if (theTemperaturePin == 0xFF)
  {
    theTemperature = 25;//Valor por defecto
    return;      
  }
  else
  {
    Temperature();
  }
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
    theTemperature = 25;//TBD
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
  
  theTemperature = (int) (celsius * 2.0); //Resolución de 0,5 grados.
  Serial.print(celsius);
}

/*
 * Procedure ANSWER (LUX)
 *    Aquí se devuelve la cantidad de lux leída.
 */
unsigned long AnswerLux(answer_t aAction)
{
   return theTemperature; //TBD
}
