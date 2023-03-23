#pragma once

#include <Arduino.h>
#include <SD.h>
#include "DateTime.h"
#include <string>

class Logger
{
public:
    Logger() = default;

    static const char *LEVEL_INFO;
    static const char *LEVEL_DEBUG;
    static const char *LEVEL_ERROR;

    void init(SDClass &sdObject, const char *logFilePath, DateTime &dtHelper)
    {
        sd = &sdObject;
        filepath = logFilePath;
        dateTimeHelper = &dtHelper;
    }

    void info(const char *message)
    {
        log(message, LEVEL_INFO);
    }

    void debug(const char *message)
    {
        log(message, LEVEL_DEBUG);
    }

    void error(const char *message)
    {
        log(message, LEVEL_ERROR);
    }

    void log(const char *message, const char *level)
    {
        dateTimeHelper->ntpUpdateTime();

        std::string dateString(dateTimeHelper->getDateString());
        std::string timeString(dateTimeHelper->getTimeString());
        std::string levelString(level);
        std::string preparedMessage(message);
        preparedMessage = dateString + " " + timeString + " [" + levelString + "] " + preparedMessage;

        Serial.println(preparedMessage.c_str());
        std::string filepathWithDate(filepath);
        // replace `.log` to `_[date].log`
        filepathWithDate.replace(filepathWithDate.find_last_of('.'), 4, "_" + dateString + ".log");
        File file = sd->open(filepathWithDate.c_str(), FILE_WRITE);

        if (!file)
        {
            Serial.printf("Error opening log file %s\n", filepathWithDate.c_str());
            return;
        }

        file.println(preparedMessage.c_str());
        file.close();
    }

private:
    SDClass *sd;
    const char *filepath;
    DateTime *dateTimeHelper;
};

const char *Logger::LEVEL_INFO = "INFO";
const char *Logger::LEVEL_DEBUG = "DEBUG";
const char *Logger::LEVEL_ERROR = "ERROR";
