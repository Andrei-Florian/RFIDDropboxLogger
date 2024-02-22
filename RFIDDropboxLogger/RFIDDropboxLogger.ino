#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883

#define AIO_USERNAME ""
#define AIO_KEY ""
#define AIO_Publish_Feed "/feeds/sendToDropbox"
#define AIO_Subscribe_Feed "/feeds/subscribeA"

#include <Bridge.h>
#include <Console.h>
#include <BridgeClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define SS_PIN 10
#define RST_PIN 5

bool debugging = true;

BridgeClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Initialise Feeds
Adafruit_MQTT_Publish sendToDropbox = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME AIO_Publish_Feed);
Adafruit_MQTT_Subscribe subscribeA = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME AIO_Subscribe_Feed);

void MQTTConnect()
{
	int8_t ret;

	if (mqtt.connected()) // if already connected
	{
		return;
	}

	if (debugging == true)
	{
		Serial.println("Connecting to Server");
	}

	while ((ret = mqtt.connect()) != 0) // attempt to connect
	{
		if (debugging == true)
		{
			Serial.print("  Error - ");
			Serial.println(mqtt.connectErrorString(ret));
			Serial.println("  Attempting Reconnection in 5 seconds");
		}

		mqtt.disconnect();
		delay(5000);
	}

	if (debugging == true)
	{
		Serial.println("  Success - Connection Established");
		Serial.println("Scan Card");
		Serial.println("");
	}
}

void setup()
{
	Bridge.begin();
	Console.begin();
	SPI.begin();

	if (debugging == true)
	{
		Serial.begin(9600);
		while (!Serial)
		{
		};
	}

	mfrc522.PCD_Init();
	delay(500);
	mqtt.subscribe(&subscribeA); // start subscription
}

void loop()
{
	MQTTConnect(); // connect to service

	Adafruit_MQTT_Subscribe *subscription;

	if (!mfrc522.PICC_IsNewCardPresent())
	{
		return;
	}
	if (!mfrc522.PICC_ReadCardSerial())
	{
		return;
	}

	String content = ""; // string to store card id

	// store card id
	for (byte i = 0; i < mfrc522.uid.size; i++)
	{
		// store the card id number in a variable
		content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
		content.concat(String(mfrc522.uid.uidByte[i], HEX));
	}

	if (debugging == true)
	{
		Serial.print("Publishing ");
		Serial.println(content);
	}

	// convert the string to a char array
	char valueToSend[content.length() + 1];
	content.toCharArray(valueToSend, content.length() + 1);

	if (!sendToDropbox.publish(valueToSend))
	{
		if (debugging == true)
		{
			Serial.println(F("  Error - Failed to Send Data"));
		}
	}
	else
	{
		if (debugging == true)
		{
			Serial.println(F("  Success - Data Sent"));
		}
	}

	if (!mqtt.ping()) // ping to keep server connection alive
	{
		if (debugging == true)
		{
			Serial.println(F("Error - Ping Failed"));
		}
	}

	delay(500);
}
