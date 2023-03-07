#include <Arduino.h>
#include "../include/Led.h"

Led::Led(unsigned int LedPin)
{
    pin = LedPin;
    pinMode(pin, OUTPUT);
}

void Led::turnOn()
{
    digitalWrite(pin, LOW);
}

void Led::turnOff()
{
    digitalWrite(pin, HIGH);
}

void Led::blink(int onTime, int offTime)
{
    turnOn();
    delay(onTime);
    turnOff();
    if (offTime > 0)
    {
        delay(offTime);
    }
}

void Led::blink(int onTime, int offTime, int times)
{
    for (int i = 0; i < times; i++)
    {
        blink(onTime, offTime);
    }
}
