/*
 *  This sketch sends sensor data via HTTP GET requests to Amazon server, RatSaaS
 *  Sensor: Si7020 temparature and humidity
 *  8/8/2015, AK, based on PB code DHT22
 *
 */

#include <ESP8266WiFi.h>
#include <Wire.h>
#include "Si7020.h"

Si7020 sensor;

// This is for the ESP8266 processor on ESP-01
float humidity, temp_f; // Values read from sensor
String webString1=""; // String to display
String webString2=""; // String to display
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0; // will store last temp was read
const long interval = 2000; // interval at which to read sensor


extern "C" {
    uint16 readvdd33(void);
}

const char* ssid     = "ssid"; //SSID
const char* password = "PW"; //Password
const char* host = "server_IP"; //Amazon server instance IP address (or domain name)
int sda = 0;//Setup I2C for ESP8266, sda pin is pin 0
int scl = 2; //scl pin is pin 2

void setup() {

    Serial.begin(115200);
    delay(10);

    // Connect to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
 
    /* WiFi.disconnect(); */
    WiFi.mode(WIFI_AP_STA); 
    WiFi.begin(ssid, password);
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print("#");
    }

    Serial.println("");
    Serial.println("WiFi connected"); 
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  
    //Initialize I2C for ESP8266, Setup pin numbers for sda and scl
    Wire.pins(sda, scl); //Setup I2C pins for ESP8266
    Wire.begin();
}

int value = 0;

void loop() {
    delay(5000);
    ++value;

    Serial.print("connecting to ");
    Serial.println(host);
 
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    webString1=" Temp: "+String((int)temp_f)+" F"; // Arduino has a hard time with float to string
    Serial.print(webString1);
    gettemperature(); // read sensor
    webString2=" Humidity: "+String((int)humidity)+"%";
    Serial.println(webString2);
 
    float vdd = readvdd33() / 1000.0;
 
    // We now create a URL for the POST
    String url  = "POST /ratsaas/post.php HTTP/1.1\r\nHost:54.149.23.69\r\nUser-Agent:Mozilla/5.0\r\n";
    url += "Accept:text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n";
    url += "Accept-Language:en-US,en;q=0.5\r\n";
    url += "Accept-Encoding:gzip,deflate\r\n";
    url +="Connection:keep-alive\r\n";
    url +="Content-Type:application/x-www-form-urlencoded\r\n";
    url +="Content-Length:180\r\n\r\n";
    url +="device_id=";
    url +="0001";
    url += "&temperature=";
    url +=String((int)temp_f);
    url +="&humidity=";
    url +=String((int)humidity);
    url +="&battery=";
    url +=String(vdd);
    url +="&GPIO0=";
    url +=String(digitalRead(0));
    url +="&GPIO2=";
    url +=String(digitalRead(2));
    url +="&Analog0=";
    url +=String(analogRead(A0));
 
 
    Serial.print("Requesting URL: ");
    Serial.println(url);
 
    // This will send the request to the server
    client.print( url );
    delay(100);
 
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }
 
    Serial.println();
    Serial.println("closing connection");
    delay(900000);
}

void gettemperature() {
    // Wait at least 2 seconds seconds between measurements.
    // if the difference between the current time and last time you read
    // the sensor is bigger than the interval you set, read the sensor
    // Works better than delay for things happening elsewhere also
    unsigned long currentMillis = millis();
    if(currentMillis - previousMillis >= interval) {
        // save the last time you read the sensor
        previousMillis = currentMillis;
	
        // Measure RH
        humidity = sensor.getRH();
        // Measure Temperature
        temp_f = sensor.getTemp();
  
        // Temperature is measured every time RH is requested. It is faster, therefore, to read it from previous RH
        // measurement instead with readTemp() float t = sensor.readTemp();
	
	    // To play switch on/off onboard heater use heaterOn() and heaterOff()
        // heaterOn();
        // delay(5000);
        // heaterOff();
    
        // Check if any reads failed and exit early (to try again).
        if (isnan(humidity) || isnan(temp_f)) {
            Serial.println("Failed to read from DHT sensor!");
            return;
            }
    }
}
