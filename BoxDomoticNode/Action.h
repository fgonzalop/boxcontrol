#ifndef action_h
#define action_h

#define MAX_ACTIONS 10

const int SET_ID_ACTION               = 0x01;
    const int REQUEST_TEMPERATURE_ACTION  = 0x02;
    const int REQUEST_LUX_ACTION          = 0x03;
    const int REQUEST_PIR_ACTION          = 0x04;
    const int REQUEST_ON_RELAY_ACTION     = 0x05;
    const int REQUEST_OFF_RELAY_ACTION    = 0x06;
    const int REQUEST_RELAY_STATUS_ACTION = 0x07;

	  unsigned long Answer(unsigned long aAction1, unsigned long aAction2);
    void Perform(unsigned long aAction1, unsigned long aAction2);



#endif
