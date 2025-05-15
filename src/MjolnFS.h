#ifndef MJOLNFS_H
#define MJOLNFS_H

#ifdef __cplusplus

#include "MjolnFS.h"
#include "MjolnConst.h"
#include "FS_BootSector.h"
#include "FileSystemManager.h"
#include "FS_FATEntry.h"
#include <Wire.h>
#include <Arduino.h>

class MjolnFileSystem
{
public:
    MjolnFileSystem(uint8_t deviceAddress);

    bool begin();
    // bool createFile(const char *filename);
    // bool writeFile(const char *filename, const uint8_t *data, uint32_t length);
    // uint32_t readFile(const char *filename, uint8_t *buffer, uint32_t maxLength);
    // bool deleteFile(const char *filename);
    // void listFiles();
    // void printFileSystemInfo();
    // void printFileInfo(const char *filename);
    bool format();
    void showLogs(bool show);

private:
    uint8_t _deviceAddress;            // I2C address of the EEPROM
    uint32_t _eepromSize;              // Size of the EEPROM in bytes
    uint16_t _pageSize;                // Size of a page in EEPROM
    const char *signature = "MjolnFS"; // File system signature

    FS_BootSector _bootSector;
    FS_FATEntry *_fatEntries;
    uint16_t _fatEntryCount;

    FS_BootSector readBootSector();
    void writeBootSector(const FS_BootSector &bootSector);
    FS_FATEntry readFATEntry(uint16_t index);
    void writeFATEntry(uint16_t index, const FS_FATEntry &entry);
};

#endif // __cplusplus
#endif // MJOLNFS_C
