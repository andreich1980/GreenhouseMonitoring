#include <Arduino.h>
#include <WiFiManager.h>
#include "Led.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

WiFiManager wifiManager;

Led led(LED_BUILTIN);

WiFiUDP ntpUDP;
// Time offset: 3:00 (in seconds)
// Sync time with server interval: 1 hour (in milliseconds)
NTPClient timeClient(ntpUDP, "ru.pool.ntp.org", 3 * 60 * 60, 1 * 60 * 60 * 1000);

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");

  wifiManager.autoConnect("GH-MONITORING", "CUCUMBERS");

  timeClient.begin();
}

void loop()
{
  led.blink(50);
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  delay(5000);
}
