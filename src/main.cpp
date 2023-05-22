#include <Arduino.h>
#include <WiFiManager.h>
#include "Led.h"
#include <DHT.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <FastBot.h>
#include <ThingSpeak.h>

WiFiManager wifiManager;
void initWiFiManager();

Led led(D2);

// DHT sensor
uint8_t DHT_PIN = D1;
DHT dht(DHT_PIN, DHT11);
byte temperature = 0;
byte humidity = 0;
void initDhtSensor();

// File system
void initFS();

struct Config
{
  char telegramBotToken[50];
  char telegramChatId[10];
  unsigned int minTemperature;
  unsigned int maxTemperature;
  unsigned long thingSpeakChannelId;
  char thingSpeakChannelWriteApiKey[17];
};
Config config;
const char *configFileName = "/config.json";
void loadConfig();
void saveConfig();

esp8266::polledTimeout::periodicMs timeout_20s(20 * 1000);
esp8266::polledTimeout::periodicMs timeout_10min(10 * 60 * 1000);
esp8266::polledTimeout::periodicMs timeout_30min(30 * 60 * 1000);

FastBot bot;
void initTelegramBot();
void telegramNotifications(byte temperature, byte humidity);
void telegramNotifications(byte temperature, byte humidity, bool forceSend);
void telegramProcessIncomingMessages(FB_msg &message);

// ThingSpeak
WiFiClient client;
void sendToThingSpeak(byte temperature, byte humidity);

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");

  initWiFiManager();

  initDhtSensor();
  initFS();

  loadConfig();

  initTelegramBot();

  ThingSpeak.begin(client);

  led.blink(50, 100, 5);
}

void loop()
{
  bot.tick();

  if (timeout_30min)
  {
    Serial.println("Reading sensors to check the battery level...");
    temperature = (int)dht.readTemperature();
    humidity = (int)dht.readHumidity();

    if (temperature == 255 && humidity == 255)
    {
      telegramNotifications(temperature, humidity, true);
    }
  }

  if (timeout_10min)
  {
    Serial.println("Reading sensors...");
    temperature = (int)dht.readTemperature();
    humidity = (int)dht.readHumidity();

    sendToThingSpeak(temperature, humidity);

    telegramNotifications(temperature, humidity);
  }

  if (timeout_20s)
  {
    led.blink(50);
  }
}

void initWiFiManager()
{
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

void initFS()
{
  Serial.println("Init LittleFS...");
  if (!LittleFS.begin())
  {
    Serial.println("[ERROR] Error mounting LittleFS.");
    return;
  }
}

void loadConfig()
{
  File file = LittleFS.open(configFileName, "r");
  if (!file)
  {
    Serial.println("Failed to open config file.");
    return;
  }

  StaticJsonDocument<384> doc;

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println("Failed to read config file, using default configuration.");
    Serial.println(error.c_str());
  }
  else
  {
    Serial.println("Loaded config file.");
  }

  file.close();

  strlcpy(config.telegramBotToken, doc["telegram_bot_token"] | "", sizeof(config.telegramBotToken));
  strlcpy(config.telegramChatId, doc["telegram_chat_id"] | "", sizeof(config.telegramChatId));
  config.minTemperature = doc["min_temperature"] | 20;
  config.maxTemperature = doc["max_temperature"] | 25;
  config.thingSpeakChannelId = doc["thingspeak_channel_id"] | 0;
  strlcpy(config.thingSpeakChannelWriteApiKey, doc["thingspeak_write_api_key"] | "", sizeof(config.thingSpeakChannelWriteApiKey));

  if (!config.telegramBotToken)
  {
    Serial.println("Telegram Bot API Token not found in the config file.");
  }

  if (!config.thingSpeakChannelWriteApiKey || !config.thingSpeakChannelWriteApiKey)
  {
    Serial.println("ThingSpeak integration data not found in the config file.");
  }
}

void saveConfig()
{
  if (LittleFS.exists(configFileName))
  {
    LittleFS.remove(configFileName);
  }

  File file = LittleFS.open(configFileName, "w");
  if (!file)
  {
    Serial.println("Failed to store config file.");
    return;
  }

  StaticJsonDocument<384> doc;

  doc["telegram_bot_token"] = config.telegramBotToken;
  doc["telegram_chat_id"] = config.telegramChatId;
  doc["min_temperature"] = config.minTemperature;
  doc["max_temperature"] = config.maxTemperature;
  doc["thingspeak_channel_id"] = config.thingSpeakChannelId;
  doc["thingspeak_write_api_key"] = config.thingSpeakChannelWriteApiKey;

  if (serializeJsonPretty(doc, file))
  {
    Serial.println("Config file stored");
  }
  else
  {
    Serial.println("Failed to store config file.");
  }

  file.close();
}

void initTelegramBot()
{
  bot.setToken(config.telegramBotToken);
  bot.setChatID(config.telegramChatId);
  bot.attach(telegramProcessIncomingMessages);

  bot.sendMessage("Greenhouse Tracker is up and running.");

  // commands list format: [{"command":"status","description":"Get current status"}]}
  // dont' forget to escape double quotes
  uint8 result = bot.sendCommand("/setMyCommands?commands=[{\"command\":\"status\",\"description\":\"Get current status\"},{\"command\":\"url\",\"description\":\"Show web-interface URL\"}]");
  Serial.print("Setting bot commands: ");
  Serial.println(result);
}

void telegramNotifications(byte temperature, byte humidity)
{
  telegramNotifications(temperature, humidity, false);
}

void telegramNotifications(byte temperature, byte humidity, bool forceSend)
{
  if (temperature == 255 && humidity == 255 && forceSend)
  {
    bot.sendMessage("🪫 The battery is running low and requires replacement");
    return;
  }

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
  else if (forceSend)
  {
    bot.sendMessage(data);
  }
}

void telegramProcessIncomingMessages(FB_msg &message)
{
  Serial.print("Telegram incoming message from ");
  Serial.print(message.username);
  Serial.print(": ");
  Serial.println(message.text);

  if (message.text == "/url")
  {
    bot.sendMessage("Web-interface URL is https://smart-home-interface.netlify.app");
    uint32 messageId = bot.lastBotMsg();
    bot.pinMessage(messageId);
  }

  if (message.text == "/status")
  {
    Serial.println("Reading sensors...");

    temperature = (int)dht.readTemperature();
    humidity = (int)dht.readHumidity();

    telegramNotifications(temperature, humidity, true);
  }
}

void sendToThingSpeak(byte temperature, byte humidity)
{
  if (!config.thingSpeakChannelId)
  {
    return;
  }

  Serial.println("ThingSpeak: sending data... ");

  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);

  int responseCode = ThingSpeak.writeFields(config.thingSpeakChannelId, config.thingSpeakChannelWriteApiKey);
  if (responseCode == 200)
  {
    Serial.println("ThingSpeak: Data sent.");
    return;
  }

  char errorMsg[50];
  snprintf(errorMsg, sizeof(errorMsg), "ThingSpeak: Failed. HTTP error code %d", responseCode);
  Serial.println(errorMsg);
}
