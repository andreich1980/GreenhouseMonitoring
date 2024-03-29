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

Led led(D6);

// DHT sensor
uint8_t DHT_POWER_PIN = D1;
uint8_t DHT_DATA_PIN = D2;
struct DhtSensorData
{
  unsigned int temperature : 8; // 8 bits (range: 0-255)
  unsigned int humidity : 8;
};
DhtSensorData readDhtSensor();
void processDhtSensorData(DhtSensorData);

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
const char *configFilename = "/config.json";
void loadConfig();
void saveConfig();

// Telegram bot
FastBot bot;
void initTelegramBot(bool isWakeUp);
struct TelegramNotificationConfig
{
  bool shouldMuteColdNotifications;
  bool shouldMuteHotNotifications;
  DhtSensorData prevDhtSensorData;
};
TelegramNotificationConfig telegramNotificationsConfig;
const char *telegramNotificationsConfigFilename = "/tg_config.json";
void loadTelegramNotificationsConfig();
void saveTelegramNotificationsConfig();
void telegramNotifications(DhtSensorData);
void telegramNotifications(DhtSensorData, bool forceSend);
void telegramProcessIncomingMessages(FB_msg &message);

// ThingSpeak
WiFiClient client;
void sendToThingSpeak(DhtSensorData sensorData);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    // Waiting for Serial to be ready...
  }
  delay(1000);
  Serial.println();

  bool isWakeUp = false;
  struct rst_info *resetInfo = system_get_rst_info();
  if (resetInfo->reason == REASON_DEEP_SLEEP_AWAKE)
  {
    Serial.println("Device woke up...");
    isWakeUp = true;
  }
  else
  {
    Serial.println("Device started...");
  }

  initFS();
  loadConfig();

  initWiFiManager();
  loadTelegramNotificationsConfig();
  initTelegramBot(isWakeUp);

  led.blink(100, 50, 3);

  DhtSensorData sensorData = readDhtSensor();
  processDhtSensorData(sensorData);

  Serial.println("Going to sleep...");
  // The chip will make RF calibration after waking up from Deep-sleep. Power consumption is high.
  system_deep_sleep_set_option(1);
  // Sleep for 10 minutes (in microseconds)
  ESP.deepSleep(600e6);
}

void loop()
{
  //
}

void initWiFiManager()
{
  wifiManager.setBreakAfterConfig(true);
  wifiManager.setConfigPortalTimeout(60);
  wifiManager.autoConnect("GH-MONITORING", "CUCUMBERS");
}

DhtSensorData readDhtSensor()
{
  Serial.println("Initializing DHT sensor...");
  pinMode(DHT_POWER_PIN, OUTPUT);
  pinMode(DHT_DATA_PIN, INPUT);

  digitalWrite(DHT_POWER_PIN, HIGH);
  delay(1000);
  DHT dht(DHT_DATA_PIN, DHT11);
  dht.begin();

  Serial.println("Reading sensor...");
  DhtSensorData data;
  data.temperature = (int)dht.readTemperature();
  data.humidity = (int)dht.readHumidity();
  Serial.print(data.temperature);
  Serial.print("°C, ");
  Serial.print(data.humidity);
  Serial.println("%");

  digitalWrite(DHT_POWER_PIN, LOW);

  return data;
}

void processDhtSensorData(DhtSensorData sensorData)
{
  // Low battery - send Telegram notification
  if (sensorData.temperature == 255 && sensorData.humidity == 255)
  {
    telegramNotifications(sensorData, true);
  }
  // Otherwise, send data to ThingSpeak API, only send Telegram notification about extreme temperatures.
  else
  {
    ThingSpeak.begin(client);
    sendToThingSpeak(sensorData);

    telegramNotifications(sensorData);
  }
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
  File file = LittleFS.open(configFilename, "r");
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
  if (LittleFS.exists(configFilename))
  {
    LittleFS.remove(configFilename);
  }

  File file = LittleFS.open(configFilename, "w");
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

void initTelegramBot(bool isWakeUp)
{
  Serial.println("Initializing Telegram bot...");
  bot.setToken(config.telegramBotToken);
  bot.setChatID(config.telegramChatId);
  bot.attach(telegramProcessIncomingMessages);
  bot.tick();

  if (!isWakeUp)
  {
    bot.sendMessage("Greenhouse Tracker is up and running.");
  }

  uint8 result;

  Serial.print("Setting bot commands... ");
  // commands list format: [{"command":"status","description":"Get current status"}]}
  // dont' forget to escape double quotes
  result = bot.sendCommand("/setMyCommands?commands=["
                           "{\"command\":\"start\",\"description\":\"Subscribe to notifications about extreme temperatures\"},"
                           "{\"command\":\"mute_cold_notifications\",\"description\":\"Mute notifications about temperature is too low\"},"
                           "{\"command\":\"mute_hot_notifications\",\"description\":\"Mute notifications about temperature is too high\"},"
                           "{\"command\":\"help\",\"description\":\"Commands description\"}"
                           "]");
  if (result == 1)
  {
    Serial.println("OK");
  }
  else
  {
    Serial.print("Error code ");
    Serial.println(result);
  }
}

void loadTelegramNotificationsConfig()
{
  File file = LittleFS.open(telegramNotificationsConfigFilename, "r");
  if (!file)
  {
    Serial.println("Failed to open Telegram notifications config file.");
    return;
  }

  StaticJsonDocument<192> doc;

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println("Failed to read Telegram notifications config file, using default configuration.");
    Serial.println(error.c_str());
  }
  else
  {
    Serial.println("Loaded Telegram notifications config file.");
  }

  file.close();

  telegramNotificationsConfig.shouldMuteColdNotifications = doc["mute_cold_notifications"] | false;
  telegramNotificationsConfig.shouldMuteHotNotifications = doc["mute_hot_notifications"] | false;
  telegramNotificationsConfig.prevDhtSensorData.temperature = doc["prev_temperature"] | 20;
  telegramNotificationsConfig.prevDhtSensorData.humidity = doc["prev_humidity"] | 40;

  Serial.print("Should mute flags: ");
  Serial.print(telegramNotificationsConfig.shouldMuteColdNotifications ? "yes, " : "no, ");
  Serial.println(telegramNotificationsConfig.shouldMuteHotNotifications ? "yes." : "no.");
}

void saveTelegramNotificationsConfig()
{
  if (LittleFS.exists(telegramNotificationsConfigFilename))
  {
    LittleFS.remove(telegramNotificationsConfigFilename);
  }

  File file = LittleFS.open(telegramNotificationsConfigFilename, "w");
  if (!file)
  {
    Serial.println("Failed to store config file.");
    return;
  }

  StaticJsonDocument<384> doc;

  doc["mute_cold_notifications"] = telegramNotificationsConfig.shouldMuteColdNotifications;
  doc["mute_hot_notifications"] = telegramNotificationsConfig.shouldMuteHotNotifications;
  doc["prev_temperature"] = telegramNotificationsConfig.prevDhtSensorData.temperature;
  doc["prev_humidity"] = telegramNotificationsConfig.prevDhtSensorData.humidity;

  if (serializeJsonPretty(doc, file))
  {
    Serial.println("Telegram notifications config file stored");
  }
  else
  {
    Serial.println("Failed to store Telegram notifications config file.");
  }

  file.close();
}

void telegramNotifications(DhtSensorData sensorData)
{
  telegramNotifications(sensorData, false);
}

void telegramNotifications(DhtSensorData sensorData, bool forceSend)
{
  if (sensorData.temperature == 255 && sensorData.humidity == 255)
  {
    bot.sendMessage("🪫 The battery is running low and requires replacement");
    return;
  }

  String data = "Temperature: " + String(sensorData.temperature) + "°C";
  data += '\n';
  data += "Humidity: " + String(sensorData.humidity) + "%";

  // Reset mute flags if the previous temperature were normal
  if (
      sensorData.temperature < config.minTemperature &&
      telegramNotificationsConfig.prevDhtSensorData.temperature >= config.minTemperature)
  {
    Serial.println("Unmute cold temperature notifications");
    telegramNotificationsConfig.shouldMuteColdNotifications = false;
  }
  if (
      sensorData.temperature > config.maxTemperature &&
      telegramNotificationsConfig.prevDhtSensorData.temperature <= config.maxTemperature)
  {
    Serial.println("Unmute hot temperature notifications");
    telegramNotificationsConfig.shouldMuteHotNotifications = false;
  }

  if (sensorData.temperature < config.minTemperature && !telegramNotificationsConfig.shouldMuteColdNotifications)
  {
    bot.sendMessage("🥶 It's too cold in the greenhouse!\n" + data);
  }
  else if (sensorData.temperature > config.maxTemperature && !telegramNotificationsConfig.shouldMuteHotNotifications)
  {
    bot.sendMessage("🥵 It's too hot in the greenhouse!\n" + data);
  }
  else if (forceSend)
  {
    bot.sendMessage(data);
  }

  telegramNotificationsConfig.prevDhtSensorData = sensorData;

  saveTelegramNotificationsConfig();
}

void telegramProcessIncomingMessages(FB_msg &message)
{
  Serial.print("Telegram incoming message from ");
  Serial.print(message.username);
  Serial.print(": ");
  Serial.println(message.text);

  if (message.text == "/start")
  {
    bot.sendMessage(
        "👋 Hi! I'm your greenhouse assistant 🌱🌡️🔔📈.\n"
        "🔔 I'll alert you when the temperature in your greenhouse gets too high/low.\n"
        "⏰I check for new messages every 10 minutes (due to power saving mode 💤).\n"
        "Let's grow some amazing plants together! 🌱💡🤖");
  }

  if (message.text == "/mute_cold_notifications")
  {
    telegramNotificationsConfig.shouldMuteColdNotifications = true;
    bot.sendMessage("Muted until the next time temperature drops below the minimum.");
  }
  else if (message.text == "/mute_hot_notifications")
  {
    telegramNotificationsConfig.shouldMuteHotNotifications = true;
    bot.sendMessage("Muted until the next time temperature rises above the maximum.");
  }

  if (message.text == "/help")
  {
    bot.sendMessage("/start - Print out an introductory message"
                    "/mute_cold_notifications - Temporarily mute notifications about low temperatures until the temperature gets back to normal and then gets too low again."
                    "/mute_hot_notifications - Temporarily mute notifications about high temperatures until the temperature gets back to normal and then gets too high again.");
  }

  saveTelegramNotificationsConfig();

  // To mark messages as read
  bot.tickManual();
}

void sendToThingSpeak(DhtSensorData sensorData)
{
  if (!config.thingSpeakChannelId)
  {
    return;
  }

  Serial.println("ThingSpeak: sending data... ");

  ThingSpeak.setField(1, (int)sensorData.temperature);
  ThingSpeak.setField(2, (int)sensorData.humidity);

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
