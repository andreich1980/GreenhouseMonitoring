#include <Arduino.h>
#include <WiFiManager.h>
#include "Led.h"
#include "DateTime.h"
#include <DHT.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "Loggger.h"

WiFiManager wifiManager;
void initWiFiManager();

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

Logger logger;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");

  initWiFiManager();

  dateTimeHelper.ntpInit();
  logger.init(SD, "/logs/working.log", dateTimeHelper);
  initDhtSensor();
  initSdCard();
}

void loop()
{
  led.blink(50);

  logger.info("Reading sensors...");
  temperature = (int)dht.readTemperature();
  humidity = (int)dht.readHumidity();

  storeData(temperature, humidity);

  delay(10 * 60 * 1000); // 10min
}

void initWiFiManager()
{
  wifiManager.setConfigPortalTimeout(60);
  wifiManager.autoConnect("GH-MONITORING", "CUCUMBERS");
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
  logger.info("Storing sensors data...");
  dateTimeHelper.ntpUpdateTime();

  char *date = dateTimeHelper.getDateString();
  char *time = dateTimeHelper.getTimeString();

  char message[40];
  sprintf(message, "Temperature: %dC, Humidity: %d%%", temperature, humidity);
  logger.info(message);

  char filePath[40];
  snprintf(filePath, 40, "/data/%s.jsonl", date);
  logger.info("Writing data to file");
  File file = SD.open(filePath, FILE_WRITE);
  if (!file)
  {
    logger.info("Failed to open file for appending");
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

  logger.info("Data saved.");

  delete[] date;
  delete[] time;
}
