/*
* BoxDomotic Node TEST (Arduino MEGA 2560)
* Igual que BoxDomoticNode_NanoTest: cada segundo activa el siguiente relé
* y envía el estado (ID, PIR, temperatura, lux, relés) por radio, sin esperar petición.
*
* Conexión nRF24L01 -> Arduino Mega
*   GND  -> GND
*   VCC  -> 3.3V  (¡nunca 5V! condensador 10-100uF entre VCC y GND)
*   CE   -> 9
*   CSN  -> 10
*   SCK  -> 52
*   MOSI -> 51
*   MISO -> 50
*   IRQ  -> sin conectar
*
* Resto de pines (igual que el Nano)
*   Relés        -> 8, 7, 6, 5, 2, A1  (activos a nivel bajo)
*   DS18B20      -> 4 (OneWire)
*   PIR          -> 3 (interrupción)
*   LDR / Lux    -> A0
*
* El Radio ID se lee de la EEPROM (bytes 0-3), grabado con BoxDomoticNode_NanoSetup.
*/
#include <OneWire.h>
#include <EEPROM.h>
#include <SPI.h>
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);  // CE, CSN

const int RADIO_ID_ADDRESS = 0;
const byte RELAY_PINS[6] = {8, 7, 6, 5, 2, A1};

unsigned long theRadioNumber = 0;
int theTemperaturePin = 4;
int thePIRPin = 3;
int theLuxPin = A0;
volatile unsigned long thePIR_START = 0;
unsigned long currentTime = 0;
byte addresses[][6] = {"BoxDo","BoxDo"};
float theTemperature = 20.0;
float voltage;
byte message[30] = {0};
byte theRelays[6] = {0};

void PIR_ISR()
{
   thePIR_START = millis();
   message[4] = 1;
}

void Temperature()
{
  OneWire ds(theTemperaturePin);
  byte data[12];
  byte addr[8];

  if (!ds.search(addr))
  {
    theTemperature = 25.0; // sin sensor
    delay(500);
    return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);
  delay(1000);
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
  for (byte i = 0; i < 9; i++) data[i] = ds.read();

  int16_t raw = (data[1] << 8) | data[0];
  byte cfg = (data[4] & 0x60);
  if (cfg == 0x00) raw = raw & ~7;       // 9 bit
  else if (cfg == 0x20) raw = raw & ~3;  // 10 bit
  else if (cfg == 0x40) raw = raw & ~1;  // 11 bit

  theTemperature = (float)raw / 16.0;
}

void Update()
{
  voltage = analogRead(theLuxPin) * (5.0 / 1023.0);
  Temperature();
  Serial.print("Lux ");
  Serial.print(voltage);
  Serial.print("  T ");
  Serial.println(theTemperature);
}

void createMessage()
{
  message[0] = theRadioNumber & 0xFF;
  message[1] = (theRadioNumber >> 8) & 0xFF;
  message[2] = (theRadioNumber >> 16) & 0xFF;
  message[3] = (theRadioNumber >> 24) & 0xFF;

  message[5] = int(theTemperature);
  message[6] = int((theTemperature - int(theTemperature)) * 100.0);
  message[7] = int(voltage);
  message[8] = int((voltage - int(voltage)) * 100.0);
  for (int i = 0; i < 6; i++) message[9 + i] = theRelays[i];
  message[16] = 0; // Time for last PIR event, TBD
  message[17] = 0;
  message[18] = 0;
  message[19] = 0;
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("***********************"));
  Serial.println(F("BoxDomotic Node MEGA 1.0.0 TEST"));
  Serial.println(F("***********************"));

  for (int i = 0; i < 4; i++)
    theRadioNumber += (unsigned long)EEPROM.read(RADIO_ID_ADDRESS + i) << (8 * i);
  Serial.print("Radio ID ");
  Serial.println(theRadioNumber);

  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);
  radio.setRetries(0, 15);
  printf_begin();
  radio.printDetails();
  radio.stopListening();

  // Relés apagados (activos a nivel bajo)
  for (int i = 0; i < 6; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    digitalWrite(RELAY_PINS[i], HIGH);
  }

  Serial.print("Configuring PIR pin ");
  Serial.print(thePIRPin);
  pinMode(thePIRPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(thePIRPin), PIR_ISR, RISING);
  Serial.println(" OK");

  Update();
}

void loop()
{
  // Recorre los 6 relés: enciende uno, envía estado, espera 1 s, lo apaga
  for (int r = 0; r < 6; r++)
  {
    if (thePIR_START != currentTime)
    {
      Serial.print("PIR... ");
      currentTime = thePIR_START;
      message[4] = 1;
    }

    Update();
    digitalWrite(RELAY_PINS[r], LOW);
    theRelays[r] = 0xFF;
    createMessage();
    bool ok = radio.write(&message, 21);
    Serial.print("TX rele ");
    Serial.print(r + 1);
    Serial.println(ok ? " OK" : " sin ACK");
    message[4] = 0;
    delay(1000);

    digitalWrite(RELAY_PINS[r], HIGH);
    theRelays[r] = 0x00;
  }
}
