#include <Arduino.h>
#include <WiFiManager.h>
#include "Led.h"
#include "DateTime.h"
#include <DHT.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

WiFiManager wifiManager;

Led led(LED_BUILTIN, true);

DateTime dateTimeHelper;

// DHT sensor
uint8_t DHT_PIN = D1;
DHT dht(DHT_PIN, DHT11);
byte temperature = 0;
byte humidity = 0;
void initDhtSensor();

// SD card
#define SD_CS (D8)
void initSdCard();
void storeData(byte temperature, byte humidity);

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");

  wifiManager.autoConnect("GH-MONITORING", "CUCUMBERS");

  dateTimeHelper.ntpInit();
  initDhtSensor();
  initSdCard();
}

void loop()
{
  led.blink(50);

  temperature = (int)dht.readTemperature();
  humidity = (int)dht.readHumidity();
  storeData(temperature, humidity);

  delay(10 * 60 * 1000); // 10min
}

void initDhtSensor()
{
  Serial.println("Init DHT sensor...");
  pinMode(DHT_PIN, INPUT);
  dht.begin();
}

void initSdCard()
{
  Serial.println("Init SD card...");
  if (!SD.begin(SD_CS))
  {
    Serial.println("SD Card mount failed.");
    return;
  }
}

void storeData(byte temperature, byte humidity)
{
  dateTimeHelper.ntpUpdateTime();

  char *date = dateTimeHelper.getDateString();
  char *time = dateTimeHelper.getTimeString();

  Serial.printf(
      "Time: %s %s, Temperature: %dC, Humidity: %d%%\n",
      date,
      dateTimeHelper.getTimeString(),
      temperature,
      humidity);

  char filePath[40];
  snprintf(filePath, 40, "/data/%s.jsonl", date);
  File file = SD.open(filePath, FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }

  char line[80];
  snprintf(line, 80,
           "{\"timestamp\":\"%s %s\",\"temperature\":%d,\"humidity\":%d}",
           date,
           dateTimeHelper.getTimeString(),
           temperature,
           humidity);

  file.println(line);
  file.close();

  delete[] date;
  delete[] time;
}
