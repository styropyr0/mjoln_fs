#ifndef MJOLNFS_C
#define MJOLNFS_C

#ifdef __cplusplus

#include "MjolnFS.h"
#include "MjolnConst.h"
#include <Wire.h>
#include <Arduino.h>

class MjolnFileSystem
{
public:
    MjolnFileSystem(uint8_t deviceAddress);

    bool begin();
    bool createFile(const char *filename);
    bool writeFile(const char *filename, const uint8_t *data, uint32_t length);
    uint32_t readFile(const char *filename, uint8_t *buffer, uint32_t maxLength);
    bool deleteFile(const char *filename);
    void listFiles();
    void printFileSystemInfo();
    void printFileInfo(const char *filename);

private:
    uint8_t _deviceAddress; // I2C address of the EEPROM
    uint32_t _eepromSize;   // Size of the EEPROM in bytes
    uint16_t _pageSize;     // Size of a page in EEPROM

    FS_BootSector _bootSector;
    FS_FATEntry *_fatEntries;
    uint16_t _fatEntryCount;

    bool eepromWriteBytes(uint32_t addr, const uint8_t *data, uint16_t length);
    bool eepromReadBytes(uint32_t addr, uint8_t *buffer, uint16_t length);

    int findFileIndex(const char *filename);
    int findFreeFATEntry();
    uint32_t allocateDataBlock(uint32_t size);
    void freeDataBlock(uint32_t startAddr, uint32_t size);

    uint32_t bytesToUint24(const uint8_t bytes[3]);
    void uint24ToBytes(uint32_t val, uint8_t bytes[3]);
    uint16_t readBootSector();
    void writeBootSector();
};

#endif // __cplusplus
#endif // MJOLNFS_C
