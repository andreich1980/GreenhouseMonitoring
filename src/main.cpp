#include <Arduino.h>
#include <WiFiManager.h>
#include "Led.h"

WiFiManager wifiManager;

Led led(LED_BUILTIN);

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");

  wifiManager.autoConnect("GH-MONITORING", "CUCUMBERS");
}

void loop()
{
  // Normal mode: short blink once in 5 seconds
  led.blink(50, 5000);
}
