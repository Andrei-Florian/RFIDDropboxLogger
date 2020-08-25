// Universum | Universum Projects > RFIDDropboxLogger

// Andrei Florian 20/FEB/2018

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "AndreiFlorian"
#define AIO_KEY         "e02f6afbb9f14755b1ad38791111e650"

#include <Bridge.h>
#include <Console.h>
#include <BridgeClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define SS_PIN 10
#define RST_PIN 5

uint32_t valueToSend; // the value to send to Adafruit IO
int proDebug = 1;

BridgeClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Initialise Feeds
Adafruit_MQTT_Publish sendToDropbox = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/sendToDropbox");
Adafruit_MQTT_Subscribe subscribeA = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/subscribeA");

void MQTTConnect()
{
  int8_t ret;

  if(mqtt.connected()) // if already connected
  {
    return;
  }

  if(proDebug == 1)
  {
    Serial.println("Connecting to Server");
  }

  while((ret = mqtt.connect()) != 0) // attempt to connect
  {
    if(proDebug == 1)
    {
      Serial.print("  Error - ");
      Serial.println(mqtt.connectErrorString(ret));
      Serial.println("  Attempting Reconnection in 5 seconds");
    }
    
    mqtt.disconnect();
    delay(5000);
  }

  if(proDebug == 1)
  {
    Serial.println("  Success - Connection Established");
    Serial.println("Scan Card");
    Serial.println("");
  }
}

void readRFID()
{
  
}

void setup()
{
  Bridge.begin();
  Console.begin();
  SPI.begin();
  
  if(proDebug == 1)
  {
    Serial.begin(9600);
    while(!Serial) {};
  }
  
  mfrc522.PCD_Init();
  delay(500);
  mqtt.subscribe(&subscribeA); // start subscription
}

void loop()
{
  MQTTConnect(); // connect to service

  Adafruit_MQTT_Subscribe *subscription;

  if(!mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  if(!mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  String content = ""; // string to store card id

  // store card id
  for(byte i = 0; i < mfrc522.uid.size; i++) 
  {
    // store the card id number in a variable
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  valueToSend++; // increase value to trigger write
  
  if(proDebug == 1)
  {
    Serial.print("Publishing ");
    Serial.println(valueToSend);
  }

  Console.println(valueToSend);
  
  if(!sendToDropbox.publish(valueToSend)) 
  {
    if(proDebug == 1)
    {
      Serial.println(F("  Error - Failed to Send Data"));
    }
  } 
  else 
  {
    if(proDebug == 1)
    {
      Serial.println(F("  Success - Data Sent"));
    }
  }

  if(! mqtt.ping()) // ping to keep server connection alive
  {
    if(proDebug == 1)
    {
      Serial.println(F("Error - Ping Failed"));
    }
  }

  delay(500);
}

