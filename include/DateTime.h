#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

class DateTime
{
private:
    unsigned long now;

    char NTP_POOL[20] = "ru.pool.ntp.org";
    int NTP_OFFSET = 3 * 60 * 60;                 // +03:00
    int NTP_UPDATE_INTERVAL = 1 * 60 * 60 * 1000; // 1 hour in ms

    WiFiUDP ntpUDP;
    NTPClient timeClient;

public:
    DateTime() : timeClient(ntpUDP)
    {
        //
    }

    /**
     * Init NTP Client
     */
    void ntpInit()
    {
        Serial.println("Init NTP Client...");

        timeClient.begin();
        timeClient.setTimeOffset(NTP_OFFSET);
        timeClient.setUpdateInterval(NTP_UPDATE_INTERVAL);
    }

    /**
     * Update time from NTP server
     */
    void ntpUpdateTime()
    {
        timeClient.update();
        now = timeClient.getEpochTime();
    }

    /**
     * Return date in YYYY-MM-DD format
     */
    char *getDateString()
    {
        setTime((time_t)timeClient.getEpochTime());

        char *date = new char[11];
        snprintf(date, 11, "%4d-%02d-%02d", year(), month(), day());

        return date;
    }

    /**
     * Return time in HH:mm:ss format
     */
    char *getTimeString()
    {
        setTime((time_t)timeClient.getEpochTime());

        char *time = new char[8];
        snprintf(time, 10, "%02d:%02d:%02d", hour(), minute(), second());

        return time;
    }
};
