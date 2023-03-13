#pragma once

#include <Arduino.h>

class Led
{
private:
    int pin;
    bool activeLow;

public:
    Led(unsigned int LedPin, bool activeLowMode = false)
    {
        pin = LedPin;
        activeLow = activeLowMode;
        pinMode(pin, OUTPUT);
    }

    void turnOn()
    {
        digitalWrite(pin, activeLow ? LOW : HIGH);
    }

    void turnOff()
    {
        digitalWrite(pin, activeLow ? HIGH : LOW);
    }

    void blink(int onTime, int offTime = 0)
    {
        turnOn();
        delay(onTime);
        turnOff();
        if (offTime > 0)
        {
            delay(offTime);
        }
    }

    void blink(int onTime, int offTime, int times)
    {
        for (int i = 0; i < times; i++)
        {
            blink(onTime, offTime);
        }
    }
};
