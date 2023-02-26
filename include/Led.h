#include <Arduino.h>
#ifndef LED_H
#define LED_H

class Led
{
public:
    Led(unsigned int LedPin);
    void turnOn();
    void turnOff();
    void blink(int onTime, int offTime = 0);
    void blink(int onTime, int offTime, int times);
};

#endif
