#pragma once

#include <Arduino.h>
#include <SD.h>
#include "DateTime.h"

class Logger
{
public:
    Logger()
    {
    }

    void init(SDClass &sdObject, const char *logFilePath, DateTime &dtHelper)
    {
        sd = sdObject;
        filepath = logFilePath;
        dateTimeHelper = dtHelper;
    }

    void info(const char *message)
    {
        dateTimeHelper.ntpUpdateTime();
        char preparedMessage[100];
        sprintf(preparedMessage, "%s %s %s", dateTimeHelper.getDateString(), dateTimeHelper.getTimeString(), message);

        Serial.println(preparedMessage);
        File file = sd.open(filepath, FILE_WRITE);

        if (!file)
        {
            Serial.printf("Error opening log file %s", filepath);
            return;
        }

        file.println(preparedMessage);
        file.close();
    }

private:
    SDClass sd;
    const char *filepath;
    DateTime dateTimeHelper;
};
