#include <Arduino.h>
#include <WiFiManager.h>
#include "Led.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <DHT.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

WiFiManager wifiManager;

Led led(LED_BUILTIN);

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ru.pool.ntp.org");
void initDhtSensor();

// DHT sensor
uint8_t DHT_PIN = D1;
DHT dht(DHT_PIN, DHT11);
byte temperature = 0;
byte humidity = 0;
void initNtpClient();

// SD card
#define SD_CS (D8)
void initSdCard();

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");

  wifiManager.autoConnect("GH-MONITORING", "CUCUMBERS");

  initNtpClient();
  initDhtSensor();
  initSdCard();
}

void loop()
{
  led.blink(50);
  timeClient.update();

  temperature = (int)dht.readTemperature();
  humidity = (int)dht.readHumidity();

  Serial.printf("Time: %s, Temperature: %dC, Humidity: %d%%\n", timeClient.getFormattedTime().c_str(), temperature, humidity);
  delay(10000);
}

void initNtpClient()
{
  timeClient.begin();
  timeClient.setTimeOffset(3 * 60 * 60);            // +03:00
  timeClient.setUpdateInterval(1 * 60 * 60 * 1000); // 1 hour in ms
}

void initDhtSensor()
{
  pinMode(DHT_PIN, INPUT);
  dht.begin();
}

void initSdCard()
{
  if (!SD.begin(SD_CS))
  {
    Serial.println("SD Card mount failed.");
    return;
  }

  Serial.println("SD Card info:");
  Serial.printf("Type: %d", SD.type());
  Serial.println();
}
