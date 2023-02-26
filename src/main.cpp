#include <Arduino.h>
#include "Blink.h"

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting");
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  // Normal mode: short blink once in 5 seconds
  Blink(50, 5000);
}
