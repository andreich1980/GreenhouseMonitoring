#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include "Led.h"
#include "DateTime.h"
#include <DHT.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include "Loggger.h"
#include <FastBot.h>

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

struct Config
{
  char hostname[20];
  char telegramBotToken[50];
  char telegramChatId[10];
  unsigned int minTemperature;
  unsigned int maxTemperature;
};
Config config;
const char *configFileName = "/config.json";
void initConfig();

ESP8266WebServer server(80);
static const char WWW_DIR[] = "/www";
static const char DATA_DIR[] = "/data";
static const char MIME_TEXT_PLAIN[] PROGMEM = "text/plain";
static const char MIME_JSON[] PROGMEM = "application/json";
void initWebServer();
void handleList();
void handleNotFound();
bool handleFileRead(String path);
void replyNotFound(String msg);
esp8266::polledTimeout::periodicMs timeout_10s(10 * 1000);
esp8266::polledTimeout::periodicMs timeout_10min(10 * 60 * 1000);

FastBot bot;
void initTelegramBot();
void telegramNotifications(byte temperature, byte humidity);

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");

  initWiFiManager();

  dateTimeHelper.ntpInit();
  logger.init(SD, "/logs/working.log", dateTimeHelper);
  initDhtSensor();
  initSdCard();

  initConfig();

  initWebServer();

  initTelegramBot();

  led.blink(50, 100, 5);
}

void loop()
{
  server.handleClient();
  MDNS.update();

  if (timeout_10min)
  {
    logger.info("Reading sensors...");
    temperature = (int)dht.readTemperature();
    humidity = (int)dht.readHumidity();

    storeData(temperature, humidity);

    telegramNotifications(temperature, humidity);
  }

  if (timeout_10s)
  {
    led.blink(50);
  }
}

void initWiFiManager()
{
  WiFiManager wifiManager;
  wifiManager.setBreakAfterConfig(true);
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

  StaticJsonDocument<100> doc;
  doc["timestamp"] = std::string(date) + " " + time;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  serializeJson(doc, file);
  file.println();

  file.close();

  logger.info("Data saved.");

  delete[] date;
  delete[] time;
}

void initConfig()
{
  File file = SD.open(configFileName);

  StaticJsonDocument<256> doc;

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    logger.error("Failed to read config file, using default configuration.");
    logger.error(error.c_str());
  }
  else
  {
    logger.info("Loaded config file.");
    // debug
    // serializeJsonPretty(doc, Serial);
    // Serial.println();
  }

  file.close();

  strlcpy(config.hostname, doc["hostname"] | "greenhouse", sizeof(config.hostname));
  strlcpy(config.telegramBotToken, doc["telegram_bot_token"] | "", sizeof(config.telegramBotToken));
  strlcpy(config.telegramChatId, doc["telegram_chat_id"] | "", sizeof(config.telegramChatId));
  config.minTemperature = doc["min_temperature"] | 16;
  config.maxTemperature = doc["max_temperature"] | 30;

  if (!config.telegramBotToken)
  {
    logger.error("Telegram Bot API Token not found in the config file.");
  }

  if (!SD.exists(configFileName))
  {
    File file = SD.open(configFileName, FILE_WRITE);
    if (!file)
    {
      logger.error("Failed to store config file.");
      return;
    }

    doc["hostname"] = config.hostname;
    doc["telegram_bot_token"] = config.telegramBotToken;
    doc["telegram_chat_id"] = config.telegramChatId;
    doc["min_temperature"] = config.minTemperature;
    doc["max_temperature"] = config.maxTemperature;
    if (serializeJsonPretty(doc, file))
    {
      logger.info("Config file stored");
    }
    else
    {
      logger.error("Failed to store config file.");
    }

    file.close();
  }
}

void initTelegramBot()
{
  bot.setToken(config.telegramBotToken);
  bot.setChatID(config.telegramChatId);

  String message = "Greenhouse Tracker is up and running.\n";
  message += "Local time is " + String(dateTimeHelper.getDateTimeString());
  bot.sendMessage(message);
}

void telegramNotifications(byte temperature, byte humidity)
{
  String data = "Temperature: " + String(temperature) + "°C";
  data += '\n';
  data += "Humidity: " + String(humidity) + "%";

  if (temperature < config.minTemperature)
  {
    bot.sendMessage("🥶 It's too cold in the greenhouse!\n" + data);
  }
  else if (temperature > config.maxTemperature)
  {
    bot.sendMessage("🥵 It's too hot in the greenhouse!\n" + data);
  }
}

void initWebServer()
{
  if (MDNS.begin(config.hostname))
  {
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS responder started");
    Serial.print("You can now connect to http://");
    Serial.print(config.hostname);
    Serial.println(".local");
    Serial.println();
  }

  server.on("/list", handleList);
  server.onNotFound(handleNotFound);
  server.enableCORS(true);

  server.begin();
  Serial.println("HTTP server started");
}

void replyNotFound(String msg)
{
  server.send(404, FPSTR(MIME_TEXT_PLAIN), msg);
}

bool handleFileRead(String path)
{
  if (!path.startsWith(DATA_DIR))
  {
    path = WWW_DIR + path;
  }

  Serial.print(String("handleFileRead: ") + path);

  if (path.endsWith("/"))
  {
    path += "index.html";
  }

  String contentType = mime::getContentType(path);
  // Prevent downloading .jsonl files
  if (path.endsWith(".jsonl"))
  {
    contentType = (String)MIME_TEXT_PLAIN;
  }
  Serial.printf(" (%s)\n", contentType.c_str());

  if (!SD.exists(path))
  {
    // File not found, try gzip version
    path = path + ".gz";
  }

  if (SD.exists(path))
  {
    File file = SD.open(path, "r");
    if (server.streamFile(file, contentType) != file.size())
    {
      Serial.println("Sent less data than expected!");
    }
    file.close();

    return true;
  }

  return false;
}

void handleList()
{
  File dataDir = SD.open(DATA_DIR);

  String list = "[";
  while (File entry = dataDir.openNextFile())
  {
    list += String("\"");
    list += entry.name();
    list += String("\",");
  }
  if (list.endsWith(","))
  {
    list = list.substring(0, list.length() - 1);
  }
  list += "]";

  server.send(200, FPSTR(MIME_JSON), list);
}

// Return file if exists, otherwise, return 404
void handleNotFound()
{
  String uri = ESP8266WebServer::urlDecode(server.uri()); // required to read paths with blanks

  if (handleFileRead(uri))
  {
    return;
  }

  // Dump debug data
  String message;
  message.reserve(100);
  message = F("Error: File not found\n\nURI: ");
  message += uri;
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += '\n';
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += F(" NAME:");
    message += server.argName(i);
    message += F("\n VALUE:");
    message += server.arg(i);
    message += '\n';
  }
  message += "path=";
  message += server.arg("path");
  message += '\n';
  Serial.print(message);

  return replyNotFound(message);
}
