#include <Arduino.h>
#include "../include/Led.h"

class Led
{
public:
    Led(unsigned int LedPin)
    {
        pin = LedPin;
        pinMode(pin, OUTPUT);
    }

    void turnOn()
    {
        digitalWrite(pin, LOW);
    }

    void turnOff()
    {
        digitalWrite(pin, HIGH);
    }

    void blink(int onTime, int offTime)
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

private:
    int pin;
};
