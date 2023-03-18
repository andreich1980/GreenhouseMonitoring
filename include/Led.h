#pragma once

#include <Arduino.h>

class Led
{
private:
    int pin;
    bool activeLow;

public:
    Led(unsigned int LedPin, bool activeLowMode = false) : pin(LedPin), activeLow(activeLowMode)
    {
        pinMode(pin, OUTPUT);
    }

    bool isOn()
    {
        return digitalRead(pin) == (activeLow ? LOW : HIGH);
    }

    void turnOn()
    {
        digitalWrite(pin, activeLow ? LOW : HIGH);
    }

    void turnOff()
    {
        digitalWrite(pin, activeLow ? HIGH : LOW);
    }

    void toggle()
    {
        if (isOn())
        {
            turnOff();
        }
        else
        {
            turnOn();
        }
    }

    void blink(int onTime, int offTime = 0, int times = 1)
    {
        for (int i = 0; i < times; i++)
        {
            turnOn();
            delay(onTime);
            turnOff();
            if (offTime > 0)
            {
                delay(offTime);
            }
        }
    }
};
