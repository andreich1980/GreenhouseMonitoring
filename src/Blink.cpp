#include <Arduino.h>
#include "../include/Blink.h"

void Blink(int onTime, int offTime, int count, int LED)
{
    for (int i = 0; i < count; i++)
    {
        digitalWrite(LED, LOW);
        delay(onTime);
        digitalWrite(LED, HIGH);
        if (offTime > 0)
        {
            delay(offTime);
        }
    }
}
