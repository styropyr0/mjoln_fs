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

enum AT24CXType
{
    AT24C04 = 0x09,
    AT24C08 = 0x0A,
    AT24C16 = 0x0B,
    AT24C32 = 0x0C,
    AT24C64 = 0x0D,
    AT24C128 = 0x0E,
    AT24C256 = 0x0F,
    AT24C512 = 0x10,
};

class MjolnFileSystem
{
public:
    MjolnFileSystem(AT24CXType eepromModel);

    bool begin();
    // bool createFile(const char *filename);
    bool writeFile(const char *filename, const char *data);
    char *readFile(const char *filename, char *buffer);
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
    AT24CXType _eepromType;            // Type of the EEPROM

    FS_BootSector _bootSector;
    FS_FATEntry *_fatEntries;
    uint16_t _fatEntryCount;

    FS_BootSector readBootSector();
    void writeBootSector(const FS_BootSector &bootSector);
    FS_FATEntry readFATEntry(uint16_t index);
    bool writeFATEntry(uint16_t index, const FS_FATEntry &entry);
    uint8_t getPageSize();
    uint16_t getUsableSize();
    uint16_t getReservedSize();
};

#endif // __cplusplus
#endif // MJOLNFS_C
