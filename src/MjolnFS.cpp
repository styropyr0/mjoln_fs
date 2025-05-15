#include "MjolnFS.h"
#include "MjolnConst.h"
#include <Wire.h>
#include <Arduino.h>

MjolnFileSystem::MjolnFileSystem(uint8_t deviceAddress)
    : _deviceAddress(deviceAddress), _eepromSize(0), _pageSize(0), _fatEntries(nullptr), _fatEntryCount(0)
{
}

FS_BootSector MjolnFileSystem::readBootSector()
{
    FS_BootSector bootSector;
    uint8_t buffer[sizeof(FS_BootSector)];
    eepromReadBytes(MJOLN_STORAGE_DEVICE_ADDRESS, 0, buffer, sizeof(FS_BootSector));
    bootSector = toBootSector(buffer);
    return bootSector;
}

void MjolnFileSystem::writeBootSector(const FS_BootSector &bootSector)
{
    uint8_t *buffer = bootSectorToBytes(&bootSector);
    eepromWriteBytes(MJOLN_STORAGE_DEVICE_ADDRESS, 0, buffer, sizeof(FS_BootSector));
    free(buffer);
}

FS_FATEntry MjolnFileSystem::readFATEntry(uint16_t index)
{
    FS_FATEntry fatEntry;
    uint8_t buffer[sizeof(FS_FATEntry)];
    eepromReadBytes(MJOLN_STORAGE_DEVICE_ADDRESS, sizeof(FS_BootSector) + index * sizeof(FS_FATEntry), buffer, sizeof(FS_FATEntry));
    fatEntry = toFATEntry(buffer, sizeof(buffer));
    return fatEntry;
}

void MjolnFileSystem::writeFATEntry(uint16_t index, const FS_FATEntry &entry)
{
    uint8_t *buffer = fatToBytes(&entry);
    eepromWriteBytes(MJOLN_STORAGE_DEVICE_ADDRESS, sizeof(FS_BootSector) + index * sizeof(FS_FATEntry), buffer, sizeof(FS_FATEntry));
    free(buffer);
}

bool MjolnFileSystem::begin()
{
    Wire.begin();
    _bootSector = readBootSector();
    if (verifyBootSector(&_bootSector))
        Serial.println("Valid file system found.");
    else
    {
        Serial.print("signature: ");
        Serial.println((int)_bootSector.signature);
        Serial.println("version: " + String(_bootSector.version));
        Serial.println("Invalid file system signature. Formatting...");
        return false;
    }
    _pageSize = _bootSector.pageSize;
    _fatEntryCount = _bootSector.fileCount[0] | (_bootSector.fileCount[1] << 8);
    return true;
}

bool MjolnFileSystem::format()
{
    if (!deletePartition(MJOLN_STORAGE_DEVICE_ADDRESS, 65535))
        return false;

    _bootSector.version = MJOLN_FILE_SYSTEM_VERSION;
    memcpy(_bootSector.signature, signature, MJOLN_FILE_SYSTEM_SIGNATURE_SIZE);
    _bootSector.pageSize = MJOLN_FILE_SYSTEM_PAGE_SIZE;
    _bootSector.fileCount[0] = 0;
    _bootSector.fileCount[1] = 0;

    writeBootSector(_bootSector);
    return true;
}