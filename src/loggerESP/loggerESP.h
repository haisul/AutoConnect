#pragma once

#ifndef __LOGGER_ESP_
#define __LOGGER_ESP_
#include <Arduino.h>

#ifndef LoggerEx
#define LoggerEx(level, format, ...) logger.printInfo_expanded(__FILE__, __LINE__, __func__, level, format, ##__VA_ARGS__)
#endif

#ifndef Logger
#define Logger(level, format, ...) logger.printInfo_contracted(__FILE__, __LINE__, __func__, level, format, ##__VA_ARGS__)
#endif

enum MessageLevel {
    LN,
    LI,
    LW,
    LE
};

class LoggerESP {
public:
    LoggerESP();

    void printInfo_expanded(const char *file, int line, const char *func, MessageLevel level, const char *format, ...);
    void printInfo_contracted(const char *file, int line, const char *func, MessageLevel level, const char *format, ...);

private:
    String formatBuf(char *buf);
    String getStyle(MessageLevel stytle);
    String getLevel(MessageLevel stytle);
    void printLog_expanded(MessageLevel stytle, const String &file, const uint16_t &line, const String &func, const String &buf);
    void printLog_contracted(MessageLevel stytle, const String &file, const uint16_t &line, const String &func, const String &buf);
};

extern LoggerESP logger;

#endif