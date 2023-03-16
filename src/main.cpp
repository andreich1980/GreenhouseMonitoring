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
#include "Loggger.h"

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

ESP8266WebServer server(80);
const char *host = "greenhouse";
static const char WWW_DIR[] = "/www";
static const char DATA_DIR[] = "/data";
static const char TEXT_PLAIN[] PROGMEM = "text/plain";
void initWebServer();
void handleNotFound();
bool handleFileRead(String path);
void replyNotFound(String msg);
esp8266::polledTimeout::periodicMs timeout_10s(10 * 1000);
esp8266::polledTimeout::periodicMs timeout_10min(10 * 60 * 1000);

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");

  initWiFiManager();

  dateTimeHelper.ntpInit();
  logger.init(SD, "/logs/working.log", dateTimeHelper);
  initDhtSensor();
  initSdCard();

  initWebServer();
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

void initWebServer()
{
  if (MDNS.begin(host))
  {
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS responder started");
    Serial.print("You can now connect to http://");
    Serial.print(host);
    Serial.println(".local");
  }

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void replyNotFound(String msg)
{
  server.send(404, FPSTR(TEXT_PLAIN), msg);
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
  Serial.printf(" (%s)\n", contentType.c_str());
  // Prevent downloading .jsonl files
  if (path.endsWith(".jsonl"))
  {
    contentType = (String)TEXT_PLAIN;
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
