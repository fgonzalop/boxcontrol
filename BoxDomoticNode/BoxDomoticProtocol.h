const int RADIO_ID_ADDRESS = 0;

const unsigned long TIMEOUT = 200;

const int SUCCESS_ANSWER = 0x00;
const int TIMEOUT_ANSWER = 0xAA;
const int NO_ANSWER      = 0xFF;

const int SET_ID_ACTION               = 0x01;
const int REQUEST_TEMPERATURE_ACTION  = 0x02;
const int REQUEST_LUX_ACTION          = 0x03;
const int REQUEST_PIR_ACTION          = 0x04;
const int REQUEST_ON_RELAY_ACTION     = 0x05;
const int REQUEST_OFF_RELAY_ACTION    = 0x06;
const int REQUEST_RELAY_STATUS_ACTION = 0x07;

typedef uint8_t byte;

struct answer_t {
  byte action1;
  byte action2;
  byte action3;
  byte action4;
};

struct payload_t {
  byte origen;
  byte messageId;
  answer_t action;
  byte spare;
  byte hop1;
  byte hop2;
  byte hop3;
  byte hop4;
  byte hop5;
  byte hop6;
  byte hop7;
};

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