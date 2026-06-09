/*
  SimpleMQTTClient.ino
  The purpose of this exemple is to illustrate a simple handling of MQTT and Wifi connection.
  Once it connects successfully to a Wifi network and a MQTT broker, it subscribe to a topic and send a message to it.
  It will also send a message delayed 5 seconds later.
*/

#include "EspMQTTClient.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(2,15);
byte addresses[][6] = {"BoxDo","BoxDo"};

EspMQTTClient client(
  "desconectada",
  "casapepe.",
  "192.168.1.103",  // MQTT Broker server ip
  //MQTTUsername",   // Can be omitted if not needed
  //"MQTTPassword",   // Can be omitted if not needed
  "TestClient",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);

void setup()
{
  Serial.begin(115200);
  Serial.println(F("***********************"));
  Serial.println(F("BoxDomotic ESP32 Testing"));
  Serial.println(F("***********************"));

  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  //radio.setRetries(5, 15);  // ARD=5, ARC=15
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
    
  // Start the radio listening for data
  radio.startListening();
  printf_begin();
  radio.printDetails();

  // Optional functionalities of EspMQTTClient
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
}

// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe("mytopic/testing", [](const String & payload) {
    Serial.println(payload);
  });

  // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
  client.subscribe("mytopic/wildcardtest/#", [](const String & topic, const String & payload) {
    byte message[30];
    int index = 0;
    
    // Parse space-separated numbers from payload
    int startIdx = 0;
    for (int i = 0; i <= payload.length() && index < 30; i++) {
      if (payload[i] == ' ' || i == payload.length()) {
        String numStr = payload.substring(startIdx, i);
        if (numStr.length() > 0) {
          message[index++] = (byte)numStr.toInt();
        }
        startIdx = i + 1;
      }
    }
    
    Serial.println("(From wildcard) topic: " + topic + ", payload: " + payload);
    Serial.printf("Parsed %i numbers\n", index);

    Serial.println("OTA: payload " + payload);

    
    if( radio.available())
    {
      radio.read(&message, 21); 
      Serial.println(message[0]);
      client.publish("mytopic/OTA", String((int)message[0]) + String((int)message[1])+ String((int)message[2]) + String((int)message[3]) +
      String((int)message[4]) + String((int)message[5])+ String((int)message[6]) + String((int)message[7]) +
      String((int)message[8]) + String((int)message[9])+ String((int)message[10]) + String((int)message[11]));
    
    }
    else
    {
      client.publish("mytopic/OTA", "ERROR");
    }
  });

  // Publish a message to "mytopic/test"
  client.publish("mytopic/testing", "Starting TESTING BoxDomotic"); // You can activate the retain flag by setting the third parameter to true

}

void loop()
{
  client.loop();
}
