#include "loggerESP.h"
LoggerESP logger;
LoggerESP::LoggerESP(){};

void LoggerESP::printInfo_expanded(const char *file, int line, const char *func, MessageLevel level, const char *format, ...) {

    va_list args;
    va_start(args, format);
    int bufSize = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);

    char buf[bufSize];

    va_start(args, format);
    vsnprintf(buf, bufSize, format, args);
    va_end(args);
    printLog_expanded(level, file, line, func, formatBuf(buf));
}

bool checkFormat(const char *format, ...) {

    char loc_buf[2];
    char *temp = loc_buf;
    va_list arg;
    va_list copy;

    va_start(arg, format);
    va_copy(copy, arg);
    int len = vsnprintf(temp, sizeof(loc_buf), format, copy);

    va_end(copy);
    va_end(arg);

    Serial.printf(".....................%d\n", len);

    if (len <= 0) {
        Serial.println("Error: Format and arguments mismatch");
        return false;
    } else {
        return true;
    }
}

void LoggerESP::printInfo_contracted(const char *file, int line, const char *func, MessageLevel level, const char *format, ...) {

    try {

        va_list args;
        va_start(args, format);
        int bufSize = vsnprintf(NULL, 0, format, args) + 1;
        va_end(args);

        char buf[bufSize];
        va_start(args, format);
        vsnprintf(buf, bufSize, format, args);
        va_end(args);
        printLog_contracted(level, file, line, func, buf);
    } catch (...) {
        printLog_contracted(LE, file, line, func, "An unknown exception occurred.");
    }
}

void LoggerESP::printLog_expanded(MessageLevel stytle, const String &file, const uint16_t &line, const String &func, const String &buf) {

    String line1 = "=", line2;
    for (int i = 0; i < 109; i++) {
        line1 += "=";
        line2 += "-";
    }
    String loggerMsg = getStyle(stytle) + line1 + "\n"
                                                  "|  #" +
                       getLevel(stytle) + "\n"
                                          "|  [" +
                       file + ":" + line + "]\n"
                                           "|  [Function:" +
                       func + "]\n"
                              "|" +
                       line2 + "\n"
                               "|\n"
                               "|  " +
                       buf + "\n"
                             "|\n" +
                       line1 + "\033[0m\n";

    Serial.println(loggerMsg);
}

void LoggerESP::printLog_contracted(MessageLevel stytle, const String &file, const uint16_t &line, const String &func, const String &buf) {
    String loggerMsg = getStyle(stytle) + "[" + getLevel(stytle) + "]" + "[" + file + ":" + line + "]" + "[" + func + "] " + buf + "\033[0m\n";
    Serial.print(loggerMsg);
}

String LoggerESP::formatBuf(char *buf) {
    const int MAX_LINE_LENGTH = 100;
    int len = strlen(buf);
    int addSize = len % MAX_LINE_LENGTH;

    for (int i = 0; i < len; i++) {
        if (buf[i] == '\n') {
            addSize += 3;
        }
        if (i > 0 && i % MAX_LINE_LENGTH == 0) {
            addSize++;
        }
    }
    addSize += 1;
    char tmp[len + addSize];
    int tmp_len = len + addSize;
    int lineCount = 0; // 紀錄當前行的字元數

    strcpy(tmp, buf);
    for (int i = 0; i < tmp_len; i++) {
        lineCount++;
        if (lineCount >= MAX_LINE_LENGTH) {
            memmove(tmp + i + 2, tmp + i + 1, tmp_len - i - 2);
            tmp[i + 1] = '\n';
            i++;
            lineCount = 0;
        }
        if (tmp[i] == '\n') {
            memmove(tmp + i + 4, tmp + i + 1, tmp_len - i - 4);
            tmp[i + 1] = '|';
            tmp[i + 2] = ' ';
            tmp[i + 3] = ' ';
            i += 4;

            lineCount = 0;
        }
    }
    tmp[tmp_len] = '\0';
    String tmpStr(tmp);
    return tmpStr;
}

String LoggerESP::getStyle(MessageLevel stytle) {
    switch (stytle) {
    case LN:
        return "\x1B[0m";
    case LI:
        return "\033[1m\x1B[34m";
    case LW:
        return "\033[33m";
    case LE:
        return "\033[1m\x1B[31m";
    default:
        return "\x1B[0m";
    }
};

String LoggerESP::getLevel(MessageLevel stytle) {
    switch (stytle) {
    case LN:
        return "N";
    case LI:
        return "I";
    case LW:
        return "W";
    case LE:
        return "E";
    default:
        return "N";
    }
};
