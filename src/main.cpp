// ******************************************************************
// Dallas Semiconductor DS18B20 Temperature Sensor Demo Program
// COMP-10184
// Mohawk College

// library for Arduino framework
#include <Arduino.h>
// 1-Wire sensor communication libary
#include <OneWire.h>
// DS18B20 sensor library
#include <DallasTemperature.h>

#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>

#include <PubSubClient.h>

const char *ssid = "Mohawk-IoT";
const char *password = "IoT@MohawK1";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Pin that the  DS18B20 is connected to
const int oneWireBus = D3;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature DS18B20(&oneWire);

WiFiServer server(80);

// interface to ThingSpeak MQTT interface
const char *mqttServer = "test.mosquitto.org"; // main
// const char* mqttServer = "broker.hivemq.com"; // alternate
const uint16_t mqttPort = 1883;
char mqttUserName[] = "";
char mqttPass[] = "";
char clientID[] = "mqtt-explorer-8dffed1d";
// WiFi client
WiFiClient client;
// MQTT publish/subscribe client

PubSubClient mqttClient(client);


int startHour;
int startMinute;
int startSecond;
long startEpoch;
int hour;
int minute;
int second;
long epoch;
long timeSince;


void subCallBack(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void updateMQTT()
{
  String acTopic = "MichaelMena/ACControl";
  String tempTopic = "MichaelMena/temp";
  String timeTopic = "MichaelMena/time";
  if (mqttClient.connected())
  {
    mqttClient.subscribe("MichaelMena/ACControl");
    float fTemp;
    DS18B20.requestTemperatures();

    // fetch the temperature.  We only have 1 sensor, so the index is 0.
    fTemp = DS18B20.getTempCByIndex(0);

    // print the temp to the USB serial monitor
    Serial.println("\nCurrent temperature is: " + String(fTemp) + " deg. Celsius");

    epoch = timeClient.getEpochTime();
    timeSince = epoch - startEpoch;

    hour = ((timeSince / 60) / 60) % 24;
    minute = (timeSince / 60) % 60;
    second = timeSince % 60;

    Serial.print("\n Time since start: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.print(minute);
    Serial.print(":");
    Serial.print(second);
    String timeSinceString = "Time since start: " + String(hour) + ":" + String(minute) + ":" + String(second);

if(timeSince > 5){}
    mqttClient.publish(tempTopic.c_str(), String(fTemp).c_str());
    mqttClient.publish(timeTopic.c_str(), String(timeSinceString).c_str());

  }
  else if (!mqttClient.connected())
  {
    mqttClient.connect(clientID, mqttUserName, mqttPass);
    
  }
}

void setup()
{
  // configure the USB serial monitor
  Serial.begin(115200);

  // Start the DS18B20 sensor
  DS18B20.begin();
  pinMode(LED_BUILTIN, OUTPUT);

  WiFi.begin(ssid, password);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Serial.printf("Web server started, open %s in a web browser\n",
                WiFi.localIP().toString().c_str());

  timeClient.begin();
  timeClient.update();

  mqttClient.setServer(mqttServer, mqttPort);

  mqttClient.setCallback(subCallBack);

  startHour = timeClient.getHours();
  startMinute = timeClient.getMinutes();
  startSecond = timeClient.getSeconds();

  startEpoch = timeClient.getEpochTime();

  Serial.println("Start time: " + String(startHour) + ":" + String(startMinute) + ":" + String(startSecond));
}



// digitalWrite(LED_BUILTIN, true);

void loop()
{
  updateMQTT();

  WiFiClient client = server.available();

  timeClient.update();

  Serial.println(timeClient.getFormattedTime());

  updateMQTT();
  mqttClient.loop();
  /*
    if (client)
    {
      Serial.println("\n>> Client connected");
      Serial.println(">> Client request:");
      while (client.connected())
      {

        // read line by line what the client (web browser) is requesting
        if (client.available())
        {
        }

        client.println("</table><footer><p>Author: Michael Mena student number 000817498<br><a href='www.github.com/pogoMena/WebServer'>Github repository</a></p></footer>");
        break;
      }
    }
    // keep read client request data (if any). After that, we can terminate
    // our client connection
    while (client.available())
    {
      client.read();
    }
    // close the connection:
    client.stop();
    Serial.println(">> Client disconnected");
    */
}
