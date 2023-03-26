#pragma once

#include <Arduino.h>

class DateTime
{
private:
    const char *NTP_POOL = "ru.pool.ntp.org";
    const char *NTP_OFFSET = "MSK-3";

public:
    DateTime() {}

    /**
     * Initialize NTP client with default parameters.
     */
    void ntpInit()
    {
        ntpInit(NTP_POOL, NTP_OFFSET);
    }

    /**
     * Initialize NTP client with custom NTP server and offset.
     * Waits until the time is synced.
     * @param server NTP server to use
     * @param offset Timezone offset from UTC in POSIX TZ format, e.g. "PST8PDT", "MSK"
     */
    void ntpInit(const char *server, const char *offset)
    {
        Serial.println("Syncing time with NTP server...");
        Serial.println("Server: " + String(server) + ", Offset: " + String(offset));

        configTime(offset, 0, server);

        Serial.print("Waiting for time to be set");
        // check if time is set after Jan 12 2001
        time_t now = time(nullptr);
        while (now < 1000000000)
        {
            delay(1000);
            Serial.print(".");
            now = time(nullptr);
        }
        Serial.println();

        struct tm *timeinfo = localtime(&now);
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%F %T", timeinfo);
        Serial.println("Time is set to " + String(timeStr));
    }

    /**
     * Return the current date in `YYYY-MM-DD` format.
     * @return A newly allocated string containing the date.
     */
    char *getDateString()
    {
        time_t now = time(nullptr);
        struct tm *timeinfo = localtime(&now);

        char *dateStr = new char[11];
        strftime(dateStr, 11, "%F", timeinfo);

        return dateStr;
    }

    /**
     * Return the current time in `HH:mm:ss` format.
     * @return A newly allocated string containing the time.
     */
    char *getTimeString()
    {
        time_t now = time(nullptr);
        struct tm *timeinfo = localtime(&now);

        char *timeStr = new char[9];
        strftime(timeStr, 9, "%T", timeinfo);

        return timeStr;
    }

    /**
     * Return the current date and time in `YYYY-MM-DD HH:mm:ss` format.
     * @return A newly allocated string containing the time.
     */
    char *getDateTimeString()
    {
        time_t now = time(nullptr);
        struct tm *timeinfo = localtime(&now);

        char *dateTimeStr = new char[20];
        strftime(dateTimeStr, 20, "%F %T", timeinfo);

        return dateTimeStr;
    }
};
