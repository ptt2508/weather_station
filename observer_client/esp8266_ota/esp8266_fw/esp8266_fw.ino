/*
  To upload through terminal you can use: curl -F "image=@firmware.bin" esp8266-webupdate.local/update
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
// library for DHT11
#include "DHT.h"
// library for MQTT
#include <PubSubClient.h>

// wifi information
const char* host = "comet-1";
const char* update_path = "/fw";
const char* update_username = "admin";
const char* update_password = "admin";
const char* ssid = "thinhp";
const char* password = "1123511235";

// MQTT server information
const char* mqtt_server = "10.96.160.109";
const uint16_t mqtt_port = 1883;

// DHT connection initialization
const int DHTPIN = 2u;
const int DHTTYPE = DHT11;
DHT dht(DHTPIN, DHTTYPE);
// structure to store temperature and humidity infromation
typedef struct
{
  float temperature;
  float humidity;
} weatherStatus_t;

// Web server initialization
ESP8266WebServer httpServer(80u);
ESP8266HTTPUpdateServer httpUpdater;

// MQTT server initialization
WiFiClient espClient;
PubSubClient client(espClient);

/********Prototype********/

/* Brief Function to read temperature and humidity from DHT11
   output: temp Temperature
   output: humi Humidity */
void tempHumObser(weatherStatus_t *wStatus);
/**/

void setup(void){
  // UART initialization
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  // connect to wifi
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  while(WiFi.waitForConnectResult() != WL_CONNECTED){
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
  }
  // Show IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  MDNS.begin(host);
  // Create web server
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);

  // start to connect to MQTT server
  client.setServer(mqtt_server, mqtt_port);
//  client.setCallback(callback);

  // start DHT
  dht.begin();
}

void loop(void)
{
  char msg[10u] = { 0u };
  weatherStatus_t wStatus = { 0u };

  httpServer.handleClient();
  if (!client.connected())
  {
    reconnect();
  }
  // Update device status
  client.publish("coWe1Stat", "1");
  tempHumObser(&wStatus);
  snprintf(msg, 10, "%0.1f,%0.1f", wStatus.temperature, wStatus.humidity);
  client.publish("coWe1",msg);
  /* Bring system to deepsleep mode for 5s */
//  system_deep_sleep(5e6);
}


void tempHumObser(weatherStatus_t *wStatus)
{
  delay(2000);
  wStatus->humidity = dht.readHumidity();
  wStatus->temperature = dht.readTemperature();
  Serial.print("Nhiet do: ");
  Serial.println(wStatus->temperature);
  Serial.print("Do am: ");
  Serial.println(wStatus->humidity);
  Serial.println();
}

void reconnect() {
  // wait until connection is success
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
