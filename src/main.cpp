#include <Arduino.h>

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting");
}

void loop()
{
  // Normal mode: short blink once in 5 seconds
  Serial.println("Blink");
  digitalWrite(LED_BUILTIN, LOW);
  delay(50);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(5000);
}
