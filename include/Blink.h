#include <Arduino.h>
#ifndef BLINK_H
#define BLINK_H

/**
 * Turn the given LED on for the given `onTime` amount of time,
 * then turn it off for the given `offTime` amount of time.
 * Repeat it `count` times
 */
void Blink(int onTime, int offTime = 0, int count = 1, int LED = LED_BUILTIN);

#endif
