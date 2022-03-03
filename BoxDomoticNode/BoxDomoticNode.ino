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
int theRelayIndex;
int theTemperaturePin;
int thePIRPin;
volatile unsigned long thePIR_START = 0;
int theLuxPin;

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

float theTemperature = 20.0;
int theRelay[MAX_RELAY] = {0,0,0,0,0,0,0,0,0,0};
void Temperature();

void setup() {
  int aIndex;
  
  Serial.begin(115200);
  Serial.println(F("*********************"));
  Serial.println(F("BoxDomotic Node 2.0.1"));
  Serial.println(F("*********************"));

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
}

/*
 * PIR_ISR
 */
void PIR_ISR()
{
   thePIR_START = millis();
}


/*
 * Procedure ANSWER
 *   Dependiendo de la ACCION se hará una llamada a una función definida
 *   
 */
answer_t Answer(answer_t aAction)
{
  answer_t aResult;
  float aTemperature;
  switch (aAction.action1)
  {
    case REQUEST_TEMPERATURE_ACTION:
       aResult.action1 = SUCCESS_ANSWER;
       aTemperature = AnswerTemperature(aAction);
       aResult.action2 = (int)(aTemperature);
       aResult.action3 = (int)((aTemperature - (int)aTemperature)*100);
Serial.print("Temperature (");
Serial.print(aAction.action2);
Serial.print(")");
Serial.println(aResult.action2);
Serial.println(aResult.action3);
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
Serial.print(theRelay[aAction.action2]);
Serial.print(" ");
Serial.println(aResult.action2);
       break;   
    case REQUEST_PIR_ACTION:
      aResult.action1 = SUCCESS_ANSWER;    
Serial.print("PIR... ");
Serial.print(thePIR_START);  
Serial.print(" ");
Serial.print((int)(millis()-thePIR_START)/1000);  
      if (thePIR_START == 0)
      {
          aResult.action2 = 0; 
      }
      else
      {
          aResult.action2 = (int)(millis()-thePIR_START)/1000;
      }
         
      thePIR_START = 0;
      break;
    case REQUEST_RELAY_STATUS_ACTION:
      aResult.action1 = SUCCESS_ANSWER;   
      aResult.action2 = 0; 
Serial.println("RELAY STATUS... ");
      int aResultado = 0;
      if (theRelayIndex <= 8)
      {
        for (int aIndex=0; aIndex<theRelayIndex; aIndex++)
        {
          aResultado= aResultado*2 + digitalRead(theRelay[aIndex]);
        } 
        aResult.action2 = aResultado;
        aResult.action3 = 0;
      }
      else
      {          
          for (int aIndex=0; aIndex<8; aIndex++)
          {
//Serial.println(aResultado);
            aResultado= aResultado*2 + digitalRead(theRelay[aIndex]);
          } 
          aResult.action2 = aResultado;
          aResult.action3 = digitalRead(theRelay[9]);   
      }
      Serial.println(aResult.action2);
      Serial.println(aResult.action3);
      
      break;
      
    default:
       aResult.action1 = NO_ANSWER;
       aResult.action2 = 0;
Serial.print("DEFAULT ( ");
Serial.print(aAction.action1);
Serial.print(" )");
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
Serial.print(" ");
Serial.print(aAction.action1);
Serial.print(" ");
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
float AnswerTemperature(answer_t aAction)
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
    theTemperature = 25.0;//Valor por defecto
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
  Serial.print(celsius);
}

/*
 * Procedure ANSWER (LUX)
 *    Aquí se devuelve la cantidad de lux leída.
 */
unsigned long AnswerLux(answer_t aAction)
{
    int sensorValue = analogRead(theLuxPin);
    // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    float voltage = sensorValue * (5.0 / 1023.0);
    // print out the value you read:
//    Serial.print(voltage);
     
    return (int)(voltage* 255.0/5.0);
}

/*
 * Procedure loop
 *    Se lee la parte de RF los datos.
 *    Se procesa y se manda ANSWER y PERFORM (si es diferido)
 */
void loop() 
{     
   if( radio.available())
   {
      radio.read(&payload_r, sizeof(payload_t)); 

      if (payload_r.hop1 == theRadioNumber)
      {    
        if (payload_r.hop2 == 0)  // mensaje directo
        {
          radio.stopListening();                                        // First, stop listening so we can talk   
          delay (10);
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
          
          radio.write( &payload_r, sizeof(payload_t) );              // Send the final one back.
          delay (10);      
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

Serial.println("Routing");
          radio.stopListening();                                        // First, stop listening so we can talk   
          delay (10);
          radio.write( &payload_r, sizeof(payload_t) );              // Send the final one back.
          delay (10);      
          radio.startListening();
          //delay(10);
        }
      }
    } 
   delay (10);
   //Serial.print(thePIR);

} // Loop
