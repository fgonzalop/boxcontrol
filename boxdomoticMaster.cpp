/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.

 03/17/2013 : Charles-Henri Hallard (http://hallard.me)
              Modified to use with Arduipi board http://hallard.me/arduipi
						  Changed to use modified bcm2835 and RF24 library
TMRh20 2014 - Updated to work with optimized RF24 Arduino library

 */

/**
 * Example RF Radio Ping Pair
 *
 * This is an example of how to use the RF24 class on RPi, communicating to an Arduino running
 * the GettingStarted sketch.
 */

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <RF24/RF24.h>

using namespace std;
//
// Hardware configuration
// Configure the appropriate pins for your connections

/****************** Raspberry Pi ***********************/

// Radio CE Pin, CSN Pin, SPI Speed
// See http://www.airspayce.com/mikem/bcm2835/group__constants.html#ga63c029bd6500167152db4e57736d0939 and the related enumerations for pin information.

// RPi generic:
RF24 radio(25, 0);

/********** User Config *********/
// Assign a unique identifier for this node, 0 or 1
bool radioNumber = 1;

/********************************/

// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t pipes[][6] = {"BoxDo", "BoxDo"};
struct payload_t {
  uint8_t origen;  
  uint8_t messageId;
  uint8_t action1;
  uint8_t action2;
  uint8_t spare;
  uint8_t hop1;
  uint8_t hop2;
  uint8_t hop3;
  uint8_t hop4;
  uint8_t hop5;
  uint8_t hop6;
  uint8_t hop7;
};

int main(int argc, char** argv)
{
	payload_t payload;
	payload_t payload_rx;
	unsigned long sent_time;
	
    cout << "BoxDomotic Gateway\n";

    // Setup and configure rf radio
    radio.begin();
radio.setPALevel(RF24_PA_LOW);
    // optionally, increase the delay between retries & # of retries
    radio.setRetries(15, 15);
    // Dump the configuration of the rf unit for debugging
    radio.printDetails();

    /***********************************/
    // This simple sketch opens two pipes for these two nodes to communicate
    // back and forth.
	
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1, pipes[1]);


    radio.startListening();

	payload.origen  = 0xFF;
	payload.messageId = 4;
	payload.action1 = 0x02;
	payload.action2 = 99;
	payload.hop1 = 2;
	payload.hop2 = 0;
	payload.hop3 = 0;
	payload.hop4 = 0;
	payload.hop5 = 0;
	payload.hop6 = 0;
	payload.hop7 = 0;
        payload.spare = 0;	

    // forever loop
    {       
		// First, stop listening so we can talk.
		radio.stopListening();

		// Take the time, and send it.  This will block until complete

		printf("Now sending...");
		sent_time = millis();

		bool ok = radio.write(&payload, sizeof(payload_t));

		if (!ok) {
			printf("failed.\n");
		}
		// Now, continue listening
		radio.startListening();

		int isAnswer = 0;
		while (!isAnswer)
		{
			// Wait here until we get a response, or timeout (250ms)
			unsigned long started_waiting_at = millis();
			bool timeout = false;
			while (!radio.available() && !timeout) {
				if (millis() - started_waiting_at > 200) {
					timeout = true;
				}
			}

			// Describe the results
			if (timeout) 
			{
				printf("Failed, response timed out.\n");
			}else 
			{
				radio.read(&payload_rx, sizeof(payload_t));

				// Spew it
				printf("Got response round-trip delay: %lu\n", millis() - sent_time);
				printf("Got payload(%d) IP= %d. msgId=%d. act1=%d.act2=%d.spa=%d.hop1=%d.hop2=%d.hop3=%d.hop4=%d.hop5=%d.hop6=%d.hop7=%d\n", sizeof(payload_t), 
						payload_rx.origen,
						payload_rx.messageId, 
						payload_rx.action1, 
						payload_rx.action2,
						payload_rx.spare,
						payload_rx.hop1,
						payload_rx.hop2,
						payload_rx.hop3,
						payload_rx.hop4,
						payload_rx.hop5,
						payload_rx.hop6,
						payload_rx.hop7
						);
						
				if (payload_rx.hop1 == 0xFF)
				{
					isAnswer = 1;
				}
			}
		}		
		sleep(1);

    } // forever loop

    return 0;
}

