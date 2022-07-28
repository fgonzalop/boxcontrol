//#include <ESP32HTTPUpdateServer.h>
#include <EspMQTTClient.h>

#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>
#include "ArduinoJson.h"
#include "BoxDomoticProtocol.h"

/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
//#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "Desconectada";
const char* password = "casa pepe.";
const char* mqtt_server = "192.168.100.199";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[51];
int value = 0;
/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 */
RF24 radio(2,4);
payload_t payload_r;
byte addresses[][6] = {"BoxDo","BoxDo"};
int theCurrentMessage = 0;
int theMaxMessages;
payload_t theRouting[10];
int theRadioNumber = 8;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("*********************");
  Serial.println("BoxDomotic Master 1.0.0");
  Serial.println("*********************");
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  radio.begin();

  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.setPALevel(RF24_PA_MAX);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
    
  // Start the radio listening for data
  //radio.startListening();
  printf_begin();
  radio.printDetails();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (payload[1] == 'f')
  {
    theRouting[3].action.action1 = REQUEST_ON_RELAY_ACTION; 
    theRouting[3].action.action2 = 1; 
  }else
  {
    theRouting[3].action.action1 = REQUEST_OFF_RELAY_ACTION; 
    theRouting[3].action.action2 = 1; 
  }
  
  
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  String IP = WiFi.localIP().toString();
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    String listOfNodes = " {\"aerotermia_1\": \"relays-a0002\", \"patioBajo\": \"box-a0004\", \"salon\": \"box-a0002\", \"lavadero\": \"box-a0009\"}"; 
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("boxdomotics/boxmaster/announce", "ESP8266 up", false);
      client.publish("boxdomotics/boxmaster/ip", IP.c_str(), false);
      client.publish("boxdomotics/boxmaster/nodes", listOfNodes.c_str(), false);
      // client.publish("boxdomotics/box-a0004/temperature", "Starting", false);
      // ... and resubscribe
      //client.subscribe("shellies/shellybutton1-C45BBE6BA510/input_event/0");
      client.subscribe("boxdomotics/relays-a0002/relay/0/command");
      client.subscribe("test/relay/1/command");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
int aIndex;
  
  aIndex = 0;
  theRouting[aIndex].messageId = 100;
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

  aIndex = 1;
  theRouting[aIndex].messageId = 102;
  theRouting[aIndex].hop1      = 9;
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
  theRouting[aIndex].hop1      = 4;
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

  aIndex = 3;
  theRouting[aIndex].messageId = 106;
  theRouting[aIndex].hop1      = 2;
  theRouting[aIndex].origen    = theRadioNumber;
  theRouting[aIndex].action.action1 = REQUEST_ON_RELAY_ACTION; 
  theRouting[aIndex].action.action2 = 1; 
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

  aIndex = 4;
  theRouting[aIndex].messageId = 108;
  theRouting[aIndex].hop1      = 2;
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

  aIndex = 5;
  theRouting[aIndex].messageId = 110;
  theRouting[aIndex].hop1      = 2;
  theRouting[aIndex].origen    = theRadioNumber;
  theRouting[aIndex].action.action1 = REQUEST_ON_RELAY_ACTION; 
  theRouting[aIndex].action.action2 = 1; 
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
  theMaxMessages = 3; 

}

void loop() {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    radio.write( &theRouting[theCurrentMessage], sizeof(payload_t) );              // Send the final one back.
    Serial.println(" OK");

    if (theCurrentMessage == 0) {
       Serial.println("should RX.. 0");
       radio.startListening();
       delay(50);
       if( radio.available()) {
          payload_r.messageId=1; //Only to enter once
          radio.read( &payload_r, sizeof(payload_t) ); 
          Serial.println("RX..");
          Serial.print(payload_r.action.action2);
          Serial.print(" ");
          Serial.print(payload_r.action.action3);
          snprintf (msg, 50, "%ld.%ld", 
                  payload_r.action.action2,
                  payload_r.action.action3);
          if (client.publish("test/temperature", msg, false))
          {
            //Serial.println("OK");
          }else
          {
            Serial.println("KO");
          }
        }
        radio.stopListening();
        delay(10);
      }

      if (theCurrentMessage == 1) {
       Serial.println("should RX.. 1");
       radio.startListening();
       delay(50);
       if( radio.available())
       {
          payload_r.messageId=1; //Only to enter once
          radio.read( &payload_r, sizeof(payload_t) ); 
          Serial.println("RX..");
          Serial.print(payload_r.action.action2);
          Serial.print(" ");
          Serial.print(payload_r.action.action3);
          snprintf (msg, 50, "%ld.%ld", 
                  payload_r.action.action2,
                  payload_r.action.action3);
          if (client.publish("test/temperature1", msg, false))
          {
            //Serial.println("OK");
          }else
          {
            Serial.println("KO");
          }
        }
        radio.stopListening();
        delay(10);
      }

      if (theCurrentMessage == 2) {
        Serial.println("should RX.. 2");
        radio.startListening();
        delay(50);
        if( radio.available()) {
          payload_r.messageId=1; //Only to enter once
          radio.read( &payload_r, sizeof(payload_t) ); 
          Serial.println("RX..");
          Serial.print(" ");
          Serial.print(payload_r.hop1);
          if (payload_r.hop1 == theRadioNumber) {
            
          }else{
            radio.read( &payload_r, sizeof(payload_t) );
            Serial.print("-> ");
            Serial.print(payload_r.hop1);
          }
          Serial.print(" ");
          Serial.print(payload_r.action.action2);
          Serial.print(" ");
          Serial.print(payload_r.action.action3);
          snprintf (msg, 50, "%ld.%ld", 
                  payload_r.action.action2,
                  payload_r.action.action3);
          if (client.publish("boxdomotics/box-a0004/temperature", msg, false))
          {
            //Serial.println("OK");
          }else
          {
            Serial.println("KO");
          }
        }
        radio.stopListening();
        delay(10);
      }
      if (theCurrentMessage == theMaxMessages) { //Máximo número de mensajes en la lista.
        theCurrentMessage = 0;
      } else {
        theCurrentMessage++;
      }
      delay (3000);
}
