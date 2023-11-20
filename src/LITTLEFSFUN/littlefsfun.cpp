#include "littlefsfun.h"

/*
   version 0.1
    Modified from example of lorol lib =2022.9.28-

   lib_deps =
   lorol/LittleFS_esp32 @ ^1.0.6


    You only need to format LITTLEFS the first time you run a
   test or else use the LITTLEFS plugin to create a partition
   https://github.com/lorol/arduino-esp32littlefs-plugin

   Warning:This example is from above link but the original
   library source has has one bug in open declared

   LisDir is display all hierarchy subdirecty
*/

bool initLittleFS() {
#ifdef ESP32
    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
#else
    if (!LittleFS.begin()) {
#endif
        Logger(LE, "Error:- LITTLEFS Mount Failed");
        return false;
    }
    return true;
}

int32_t levellist = 1;

String printdirentry(File direntry, int32_t levels) {
    String retstr = String();
    for (int32_t i = 0; i < levels; i++) {
        retstr += "\t";
    }
    if (direntry.isDirectory()) { //  *DIR
        retstr += "*";
        retstr += String(direntry.name());
    } else { // FILE
        retstr += " ";
        retstr += String(direntry.name());
        retstr += "\t";
        retstr += String(direntry.size());
#ifndef CONFIG_LITTLEFS_FOR_IDF_3_2
        time_t t = direntry.getLastWrite();
        struct tm *tmstruct = localtime(&t);
        char buffer[40];
        sprintf(buffer, "\t%d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
        retstr += String(buffer);
#endif
    }
    retstr += "\r\n";
    return retstr;
}

String dirlistContent = String();

void listDir1(fs::FS &fs, const char *dirname, uint8_t levels);

String listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
    levellist = levels;
    levels = 0;
    dirlistContent = "directory: " + String(dirname) + "\r\n";
    listDir1(fs, dirname, levels);

    return dirlistContent;
}

void listDir1(fs::FS &fs, const char *dirname, uint8_t levels) {

#ifdef ESP32
    File root = fs.open(dirname);
#else
    File root = fs.open(dirname, FILE_READ);
#endif
    if (!root) {
        dirlistContent += "*** failed to open directory\r\n";
        return;
    }
    if (!root.isDirectory()) {
        dirlistContent += printdirentry(root, levels);
        return;
    }
    File file = root.openNextFile();
    while (file) {
        dirlistContent += printdirentry(file, levels);
        if (file.isDirectory()) {
            if (levels <= levellist) {
                char *p = (char *)malloc(strlen(dirname) + 5 + strlen(file.name()));
                assert(p != NULL);
                strcpy(p, dirname);

                if (strlen(dirname) != 1) {
                    p = strcat(p, "/");
                }
                p = strcat(p, file.name());
                listDir1(fs, p, levels + 1);

                free(p);
            }
        }
        file.close();
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char *path) {
    if (!fs.mkdir(path)) {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char *path) {

    if (!fs.rmdir(path)) {
    }
}

String readFile(fs::FS &fs, const char *path) {
    Logger(LN, "Reading file: %s\r", path);
#ifdef ESP32
    File file = fs.open(path);
#else
    File file = fs.open(path, FILE_READ);
#endif
    if (!file || file.isDirectory()) {
        Logger(LE, "ERROR: Empty file or failed to open file");
        return String();
    }

    if (!file.available()) {
        Logger(LE, "ERROR: File is not available");
        file.close();
        return String();
    }

    String fileContent;
    while (file.available()) {
        fileContent += String((char)file.read());
    }
    file.close();

    Logger(LN, "File closed successfully");

    return fileContent;
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
    Logger(LN, "Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);

    if (!file) {
        Logger(LE, "- failed to open file for writing");
        return;
    }

    if (!file.print(message)) {
        Logger(LE, "- write failed");
    } else {
        Logger(LN, "- write successful");
    }

    file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {

    File file = fs.open(path, FILE_APPEND);
    if (!file) {

        return;
    }
    if (!file.print(message)) {
        Logger(LE, "- append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {

    if (!fs.rename(path1, path2)) {
        Logger(LE, "- rename failed");
    }
}

void deleteFile(fs::FS &fs, const char *path) {

    if (!fs.remove(path)) {
    }
}

// SPIFFS-like write and delete file, better use #define CONFIG_LITTLEFS_SPIFFS_COMPAT 1

void writeFile2(fs::FS &fs, const char *path, const char *message) {
    if (!fs.exists(path)) {
        if (strchr(path, '/')) {

            char *pathStr = strdup(path);
            if (pathStr) {
                char *ptr = strchr(pathStr, '/');
                while (ptr) {
                    *ptr = 0;
                    if (*pathStr != 0 && !fs.exists(pathStr)) {

                        fs.mkdir(pathStr);
                        if (!fs.exists(pathStr)) {
                        }
                    }
                    *ptr = '/';
                    ptr = strchr(ptr + 1, '/');
                }
            }
            free(pathStr);
        }
    }

    File file = fs.open(path, FILE_WRITE);
    if (!file) {

        return;
    }
    if (file.print(message)) {

    } else {
    }
    file.close();
}

void deleteFile2(fs::FS &fs, const char *path) {

    if (fs.remove(path)) {

    } else {
    }

    char *pathStr = strdup(path);
    if (pathStr) {
        char *ptr = strrchr(pathStr, '/');
        if (ptr) {
        }
        while (ptr) {
            *ptr = 0;
            fs.rmdir(pathStr);
            ptr = strrchr(pathStr, '/');
        }
        free(pathStr);
    }
}

#ifdef ESP32
#define TESTBLOCK 2048
#else
#define TESTBLOCK 1024
#endif
void testFileIO(fs::FS &fs, const char *path) {
    Logger(LI, "Testing file I/O with %s\r\n", path);

    static uint8_t buf[512];
    size_t len = 0;
    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Logger(LE, "- failed to open file for writing");
        return;
    }

    size_t i;
    Logger(LI, "- writing");
    uint32_t start = millis();
    for (i = 0; i < TESTBLOCK; i++) {
        if ((i & 0x001F) == 0x001F) {
            Serial.print(".");
        }
        file.write(buf, 512);
    }
    Serial.println("");
    uint32_t end = millis() - start;
    Logger(LI, " - %u bytes written in %u ms\r\n", TESTBLOCK * 512, end);
    file.close();

#ifdef ESP32
    file = fs.open(path);
#else
    file = fs.open(path, FILE_READ);
#endif
    start = millis();
    end = start;
    i = 0;
    if (file && (!file.isDirectory())) {
        len = file.size();
        size_t flen = len;
        start = millis();
        Logger(LI, "- reading %d", len);
        while (len) {
            size_t toRead = len;
            if (toRead > 512) {
                toRead = 512;
            }
            file.read(buf, toRead);
            if ((i++ & 0x001F) == 0x001F) {
                Serial.print(".");
            }
            len -= toRead;
        }
        Serial.println("");
        end = millis() - start;
        Logger(LI, "- %u bytes read in %u ms\r\n", flen, end);
        file.close();
    } else {
        Logger(LE, "- failed to open file for reading");
    }
}
