#pragma once

#include <Arduino.h>

class Led
{
    int pin;

public:
    Led(unsigned int LedPin);
    void turnOn();
    void turnOff();
    void blink(int onTime, int offTime = 0);
    void blink(int onTime, int offTime, int times);
};
