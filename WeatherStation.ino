#define DISABLE_DIAGNOSTIC_OUTPUT

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include "credentials.h"

#include <ArduinoJson.h>

//https://github.com/tuanpmt/ESP8266MQTTClient/blob/master/examples/MQTTClient/MQTTClient.ino
#include <ESP8266MQTTClient.h>

//https://github.com/arduino-libraries/NTPClient/blob/master/examples/Basic/Basic.ino
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <GxEPD.h>
#include <GxGDEW042T2/GxGDEW042T2.cpp>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

#include "icons.h"

#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

/**************************(Definieren der genutzten Variabeln)****************************/

String hostname = "WeatherStation01";
// MQTT
MQTTClient mqtt;

int minute = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

GxIO_Class io(SPI, SS, 0, 2);  //SPI,SS,DC,RST
GxEPD_Class display(io, 2, 4);  //io,RST,BUSY
/*****************************************( Rooms )****************************************/

float bedroomData[]     = {0, 0};
float livingroomData[]  = {0, 0};
float nurseryData[]     = {0, 0};
float kitchenData[]     = {0, 0};

/****************************************(Helper)******************************************/

void parseJsonTempDataToArray(String data, float arrayX[]) {
       // Allocate JsonBuffer, see http://arduinojson.org/assistant/
      StaticJsonBuffer<200> jsonBuffer;
      // Parse JSON object
      JsonObject& root = jsonBuffer.parseObject(data);
      if (!root.success()) {
        Serial.println("Parsing failed!");
        return;
      }
      arrayX[0] = root["DHT11"]["Temperature"].as<float>();
      arrayX[1] = root["DHT11"]["Humidity"].as<float>();
}

WiFiClient getOpenWeatherData() {
  // ToDo: Exception handling

  WiFiClient client;

  client.setTimeout(10000);
  if (!client.connect("api.openweathermap.org", 80)) {
    Serial.println("Connection failed");
  }
  // Send HTTP request
  client.println("GET /data/2.5/weather?q=Berlin&appid=" + APP_ID + "&units=metric&mode=json&lang=en HTTP/1.1");
  client.println("Host: api.openweathermap.org");
  client.println("User-Agent: ArduinoWiFi/1.1");
  client.println("Connection: close");
  if (client.println() == 0) {
    Serial.println("Failed to send request");
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print("Unexpected response: ");
    Serial.println(status);
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println("Invalid response");
  }

  // Disconnect
  //client.stop();

  return client;
}

/*****************************************( Setup )****************************************/

void setupWIFI() {

  // Set WIFI module to STA mode
  WiFi.mode(WIFI_STA);

  // Connect
  Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
  WiFi.hostname(hostname);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  WiFi.setAutoReconnect(1);

  // Connected!
  Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void setupScreen() {
  display.init();
  display.setRotation(90);                   
  display.fillScreen(GxEPD_WHITE);
  display.drawBitmap(gImage_grid, 0, 0, 400 , 250, GxEPD_BLACK);
  display.update();
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeMonoBold9pt7b);
}


void setupMQTT() {
  
  //topic, data, data is continuing
  mqtt.onData([](String topic, String data, bool cont) {
    if (topic == bedroomTopic) {
      Serial.printf("Bedroom Data received, topic: %s, data: %s\r\n", topic.c_str(), data.c_str());
      parseJsonTempDataToArray(String(data.c_str()), bedroomData);
      displayBedRoomData();

    } else if (topic == livingroomTopic) {
      Serial.printf("Livingroom Data received, topic: %s, data: %s\r\n", topic.c_str(), data.c_str());
      parseJsonTempDataToArray(String(data.c_str()), livingroomData);
      displayLivingRoomData();
      
    } else if (topic == nurseryTopic) {
      Serial.printf("Nursery Data received, topic: %s, data: %s\r\n", topic.c_str(), data.c_str());
      parseJsonTempDataToArray(String(data.c_str()), nurseryData);
      displayNurseryData();
      
    } else if (topic == kitchenTopic) {
      Serial.printf("Kitchen Data received, topic: %s, data: %s\r\n", topic.c_str(), data.c_str());
      parseJsonTempDataToArray(String(data.c_str()), kitchenData);
      displayKitchenData();
    }
  });

  mqtt.onSubscribe([](int sub_id) {
    Serial.printf("Subscribe topic id: %d ok\r\n", sub_id);
  });

  mqtt.onConnect([]() {
    // QOS 1 should let the data in the Queue
    Serial.printf("MQTT: Connected\r\n");

    mqtt.subscribe(bedroomTopic, 1);
    mqtt.subscribe(livingroomTopic, 1);
    mqtt.subscribe(nurseryTopic, 1);
    mqtt.subscribe(kitchenTopic, 1);
  });

  mqtt.begin("mqtt://" + MQTT_SERVER + ":" + MQTT_PORT + "#" + hostname);
}

void setup()
{
  Serial.begin(115200);
  
  setupWIFI();
  setupMQTT();
  setupScreen();
  
  timeClient.begin();
  timeClient.setTimeOffset(3600);
  timeClient.update();
}

/*************************************(Hauptprogramm)**************************************/

void displayWifiStatus() {
  display.fillRect(wifiBox[0], wifiBox[1], wifiBox[2], wifiBox[3], GxEPD_WHITE);
  if (WiFi.status() != WL_CONNECTED) {
    display.drawBitmap(gImage_no_wifi, 345, 255, 50 , 41, GxEPD_BLACK);
  } else {
    display.drawBitmap(gImage_wifi, 345, 255, 50 , 39, GxEPD_BLACK);
  }
}

bool displayTime() {
  if (minute != int(timeClient.getMinutes())) {
    display.fillRect(clockBox[0], clockBox[1], clockBox[2], clockBox[3], GxEPD_WHITE);
    minute = int(timeClient.getMinutes());
    display.setCursor(10, 290);
    display.setFont(&FreeMonoBold24pt7b);
    // Display time without seconds
    display.print(String(timeClient.getFormattedTime()).substring(0, 5));
    
    display.setFont(&FreeMonoBold9pt7b);
    return true;
  }
  return false;
}

void displayOpenWeatherData() {
  WiFiClient client = getOpenWeatherData();

  // Allocate JsonBuffer, see http://arduinojson.org/assistant/
  StaticJsonBuffer<1200> jsonBuffer;

  // Parse JSON object
  JsonObject& root = jsonBuffer.parseObject(client);
  if (!root.success()) {
    Serial.println("Parsing failed!");
    return;
  }

  display.fillRect(openWeatherBox[0], openWeatherBox[1], openWeatherBox[2], openWeatherBox[3], GxEPD_WHITE);
  display.setCursor(35, 15);
  display.print("Weather in Berlin");
  // Extract values
  display.setCursor(115, 50);
  display.print("TMP: ");
  display.print(root["main"]["temp"].as<float>(), 1);
  display.print(" C");

  display.setCursor(115, 70);
  display.print("Hum: ");
  display.print(root["main"]["humidity"].as<int>());
  display.print(" %");

  display.setCursor(115, 90);
  display.print("Wnd: ");
  display.print(root["wind"]["speed"].as<float>(), 0);
  display.print(" Kmh");

  displayWeatherIcon(root["weather"][0]["icon"]);
  display.setCursor(10, 130);
  display.print(root["weather"][0]["description"].as<String>());
}

void displayLivingRoomData() {
  display.fillRect(livingroomBox[0], livingroomBox[1], livingroomBox[2], livingroomBox[3], GxEPD_WHITE);
  display.setCursor(280, 15);
  display.print("Livingroom");

  display.setCursor(281, 50);
  display.print("TMP: ");
  display.print(livingroomData[0], 0);
  display.print(" C");

  display.setCursor(281, 70);
  display.print("Hum: ");
  display.print(livingroomData[1], 0);
  display.print(" %");
  
}

void displayBedRoomData() {
  display.fillRect(bedroomBox[0], bedroomBox[1], bedroomBox[2], bedroomBox[3], GxEPD_WHITE);
  display.setCursor(25, 165);
  display.print("Bedroom");

  display.setCursor(15, 203);
  display.print("TMP: ");
  display.print(bedroomData[0], 0);
  display.print(" C");

  display.setCursor(15, 223);
  display.print("Hum: ");
  display.print(bedroomData[1], 0);
  display.print(" %");
}

void displayNurseryData() {
  display.fillRect(nurseryBox[0], nurseryBox[1], nurseryBox[2], nurseryBox[3], GxEPD_WHITE);
  display.setCursor(168, 165);
  display.print("Nursery");

  display.setCursor(148, 203);
  display.print("TMP: ");
  display.print(nurseryData[0], 0);
  display.print(" C");

  display.setCursor(148, 223);
  display.print("Hum: ");
  display.print(nurseryData[1], 0);
  display.print(" %");

}

void displayKitchenData() {
  display.fillRect(kitchenBox[0], kitchenBox[1], kitchenBox[2], kitchenBox[3], GxEPD_WHITE);
  display.setCursor(301, 165);
  display.print("Kitchen");

  display.setCursor(281, 203);
  display.print("TMP: ");
  display.print(kitchenData[0], 0);
  display.print(" C");

  display.setCursor(281, 223);
  display.print("Hum: ");
  display.print(kitchenData[1], 0);
  display.print(" %");
}

void displayWeatherIcon(String weather) {
  // see https://openweathermap.org/weather-conditions
  if (weather.startsWith("01")) {
    display.drawBitmap(gImage_sun, 10, 25, 100 , 96, GxEPD_BLACK);
  } else if (weather.startsWith("02")) {
    display.drawBitmap(gImage_few_clouds, 10, 25, 100 , 95, GxEPD_BLACK);
  } else if (weather.startsWith("03")) {
    display.drawBitmap(gImage_cloudy, 10, 25, 100 , 80, GxEPD_BLACK);
  } else if (weather.startsWith("04")) {
    display.drawBitmap(gImage_cloudy, 10, 25, 100 , 80, GxEPD_BLACK);
  } else if (weather.startsWith("09")) {
    display.drawBitmap(gImage_shower_rain, 10, 25, 100 , 85, GxEPD_BLACK);
  } else if (weather.startsWith("10")) {
    display.drawBitmap(gImage_rain, 10, 25, 100 , 85, GxEPD_BLACK);
  } else if (weather.startsWith("11")) {
    display.drawBitmap(gImage_thunderstorm, 10, 25, 100 , 85, GxEPD_BLACK);
  } else if (weather.startsWith("13")) {
    display.drawBitmap(gImage_snow, 10, 25, 100 , 85, GxEPD_BLACK);
  } else if (weather.startsWith("50")) {
    display.drawBitmap(gImage_fog, 10, 25, 100 , 60, GxEPD_BLACK);
  } else {
    display.drawBitmap(gImage_unknown, 10, 25, 100 , 70, GxEPD_BLACK);
  }
}

void loop()
{
  mqtt.handle();
  timeClient.update();

  if (displayTime()) {
    displayWifiStatus();
    displayOpenWeatherData();
    display.update();
  }
  
  delay(200);

}

