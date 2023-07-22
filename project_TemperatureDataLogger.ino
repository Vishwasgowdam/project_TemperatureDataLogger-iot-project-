/************************************************************
    Example Program: NodeMCU to Google Sheets
    Program to upload data from NodeMCU to Google Sheets.
    by Vishwasgowda.M @ mvishwasgowda@gmail.com
************************************************************/


/* Add JSON Package and Install: esp8266 by ESP8266 Community */
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <OneWire.h>
/* Install: "OneWire" by Jim Studt, Tom Pollard etc., */

#include <DallasTemperature.h>
/* Install: "DallasTemperature" by Miles Burton, Tim Newsome etc., */

// Pin Numbers:
#define OneWireBus_Pin 4
#define Buzzer_Pin 14   // (Output) Connect to the I/O pin of Buzzer Module.
#define WLS_Pin A0   // (Input) Connect to S pin of Water Level Sensor.

// Configurations: Buzzer Module (LOW Level)
#define BuzzerOn LOW
#define BuzzerOff HIGH

// Objects:
WiFiClientSecure client;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(OneWireBus_Pin);

// Pass oneWire reference to Dallas Temperature Library
DallasTemperature objDT(&oneWire);

// Your Wi-Fi credentials:
const char ssid[] = "VIBE K5 Note";  // Name of your network (Hotspot or Router name)
const char pass[] = "1234567890"; // Corresponding Password

// Host and HTTPS Port:
const char host[] = "script.google.com";
const int httpsPort = 443;

// Google App Script ID:
String GAS_Deployment_ID = "AKfycbzNUx3syTsXpEiyCLw7yP9f4WZeoa8DQIavTe4b38Lj_U8jwwUSIzC52Pt6y66jJ1cUiA";
const char fingerprint[] = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";

// Variables:
// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long millisBeforeExit = 0;
unsigned long delayInterval = 1000 * 10;

int waterTemp;
int WLS_Value;

void setup() {
  // Pin Mode Configuration: Buzzer Module
  pinMode(Buzzer_Pin, OUTPUT);
  digitalWrite(Buzzer_Pin, BuzzerOff);
    
  /* Begin serial communication with Arduino and Arduino IDE (Serial Monitor) */
  Serial.begin(9600);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  client.setInsecure();

  // Start up the library:
  objDT.begin();
}

void loop() {
  WLS_Value = analogRead(WLS_Pin);
  Serial.print("WLS Value = ");
  Serial.println(WLS_Value);

  if (WLS_Value < 500) {
    Serial.print("Water is Not Present");
    digitalWrite(Buzzer_Pin, BuzzerOn);
    delay(500);
    digitalWrite(Buzzer_Pin, BuzzerOff);
    delay(500);    
  }

  if (millis() - millisBeforeExit > delayInterval) {
    Serial.println("TEMPERATURE");
    Serial.print("Celsius: ");
    waterTemp = getTempratureInC();
    Serial.println(getTempratureInC());
    Serial.print("Fahrenheit: ");
    Serial.println(getTempratureInF());
    Serial.println();

    Serial.print("Connecting to ");
    Serial.println(host);
    if (!client.connect(host, httpsPort)) {
      Serial.println("Connection failed");
      client.stop();
      return;
    }

    if (client.verify(fingerprint, host)) {
      Serial.println("Certificate Matches");
    } else {
      Serial.println("Certificate doesn't match!");
    }

    String url = "/macros/s/" + GAS_Deployment_ID + "/exec?Value1=" +
                 String(waterTemp);
    Serial.print("Requesting URL: ");
    Serial.println(url);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "User-Agent: BuildFailureDetectorESP8266\r\n" +
                 "Connection: close\r\n\r\n");
    Serial.println("Request sent.");

    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }
    client.stop();
    Serial.println("\n[Disconnected]");
    millisBeforeExit = millis();
  }
}

int getTempratureInC() {
  objDT.requestTemperatures();
  return objDT.getTempCByIndex(0);    //0 refers to the first IC on the wire
}

int getTempratureInF() {
  objDT.requestTemperatures();
  return objDT.getTempFByIndex(0);    //0 refers to the first IC on the wire
}
