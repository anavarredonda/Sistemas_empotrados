
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include <DHT.h>

#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    	 // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the WiFi radio's status

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "192.168.28.2";
int        port     = 31883;
const char topic[]  = "SDC/sensor/";
const char topic2[]  = "SDC/poten/";

DHT dht(1, DHT11);


#define FREQ_SEND_TEMPERATURE 1000
#define FREQ_SEND_POTEN 100

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(5000);
  }
  
  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();

  while (true) {

    if (!mqttClient.connect(broker, port)) {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());

      delay(1000);
    }
    else
      break;
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  dht.begin();
}

String readSensor() {

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed at reading sensor DHT11!");
    return "";
  }

  // Print to serial monitor
  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print(" % ");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.println(" *C");

  return "{ \"Humedad\" : " + String(h) + ", \"Temperature\" : " + String(t) + "}";
}

long lastTimeTemp=0;
long lastTimePoten=0;

void loop() {
  
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  if (millis() - lastTimeTemp > FREQ_SEND_TEMPERATURE ) {

    String jsonInfo = readSensor();
    mqttClient.beginMessage(topic);
    mqttClient.print(jsonInfo);
    mqttClient.endMessage();
    lastTimeTemp = millis();
    
  }

  if (millis() - lastTimePoten > FREQ_SEND_POTEN ) {
    int sensorValue = analogRead(A0);
    Serial.println("Poten: " + String(sensorValue));
    mqttClient.beginMessage(topic2);
    mqttClient.print("{ \"poten\" : " + String(sensorValue) +" }");
    mqttClient.endMessage();
    lastTimePoten=millis();
  }
}


void printWifiData() {
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
