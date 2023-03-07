#include <Arduino.h>
#include "Led.h"

Led led(LED_BUILTIN);

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");
}

void loop()
{
  // Normal mode: short blink once in 5 seconds
  led.blink(50, 5000);
}
