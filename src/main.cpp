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
// DhtSensorData prevSensorData = {
//   temperature : 0,
//   humidity : 0
// };
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
const char *configFileName = "/config.json";
void loadConfig();
void saveConfig();

// Telegram bot
FastBot bot;
void initTelegramBot(bool isWakeUp);
bool shouldMuteColdNotifications = false;
bool shouldMuteHotNotifications = false;
void telegramNotifications(DhtSensorData sensorData);
void telegramNotifications(DhtSensorData sensorData, bool forceSend);
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

  Serial.print("Should mute flags: ");
  Serial.print(shouldMuteColdNotifications ? "yes, " : "no, ");
  Serial.println(shouldMuteHotNotifications ? "yes." : "no.");

  initFS();
  loadConfig();

  initWiFiManager();
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

void initTelegramBot(bool isWakeUp)
{
  Serial.println("Initializing Telegram bot...");
  bot.setToken(config.telegramBotToken);
  bot.setChatID(config.telegramChatId);
  // bot.attach(telegramProcessIncomingMessages);
  // bot.tickManual();

  if (!isWakeUp)
  {
    bot.sendMessage("Greenhouse Tracker is up and running.");
  }

  // commands list format: [{"command":"status","description":"Get current status"}]}
  // dont' forget to escape double quotes
  // uint8 result = bot.sendCommand("/setMyCommands?commands=["
  //                                "{\"command\":\"start\",\"description\":\"Unsubscribe from all the notifications\"},"
  //                                "{\"command\":\"stop\",\"description\":\"Subscribe to notifications about extreme temperatures\"},"
  //                                "{\"command\":\"mute_cold_notifications\",\"description\":\"Mute notifications about temperature is too low\"},"
  //                                "{\"command\":\"mute_hot_notifications\",\"description\":\"Mute notifications about temperature is too high\"},"
  //                                "{\"command\":\"help\",\"description\":\"Commands description\"}"
  //                                "]");
  // Serial.print("Setting bot commands... ");
  // if (result == 1)
  // {
  //   Serial.println("OK");
  // }
  // else
  // {
  //   Serial.print("Error code ");
  //   Serial.println(result);
  // }
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

  // Reset mute flags if the previous temperature has the opposite extreme value
  // if (sensorData.temperature < config.minTemperature && prevSensorData.temperature > config.maxTemperature)
  // {
  //   Serial.println("Unmute cold temperature notifications");
  //   shouldMuteColdNotifications = false;
  // }
  // if (sensorData.temperature > config.maxTemperature && prevSensorData.temperature < config.minTemperature)
  // {
  //   Serial.println("Unmute hot temperature notifications");
  //   shouldMuteHotNotifications = false;
  // }

  if (sensorData.temperature < config.minTemperature && !shouldMuteColdNotifications)
  {
    bot.sendMessage("🥶 It's too cold in the greenhouse!\n" + data);
  }
  else if (sensorData.temperature > config.maxTemperature && !shouldMuteColdNotifications)
  {
    bot.sendMessage("🥵 It's too hot in the greenhouse!\n" + data);
  }
  else if (forceSend)
  {
    bot.sendMessage(data);
  }

  // prevSensorData = sensorData;
}

void telegramProcessIncomingMessages(FB_msg &message)
{
  Serial.print("Telegram incoming message from ");
  Serial.print(message.username);
  Serial.print(": ");
  Serial.println(message.text);

  if (message.text == "/start")
  {
    bot.sendMessage("I will notify you about extreme temperatures.");
  }

  if (message.text == "/stop")
  {
    bot.sendMessage("You will not receive any notifications from now on.");
  }

  if (message.text.startsWith("/mute_cold_notifications"))
  {
    shouldMuteColdNotifications = true;
    bot.sendMessage("Muted until the next time temperature drops below the minimum.");
  }
  else if (message.text.startsWith("/mute_hot_notifications"))
  {
    shouldMuteHotNotifications = true;
    bot.sendMessage("Muted until the next time temperature rises above the maximum.");
  }
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
