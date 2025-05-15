#include "MjolnFS.h"
#include "MjolnConst.h"
#include <Wire.h>
#include <Arduino.h>

MjolnFileSystem::MjolnFileSystem(uint8_t deviceAddress)
    : _deviceAddress(deviceAddress), _eepromSize(0), _pageSize(0), _fatEntries(nullptr), _fatEntryCount(0)
{
}

void readBootSector()
{
    uint8_t buffer[sizeof(FS_BootSector)];
    if (eepromReadBytes(0, buffer, sizeof(FS_BootSector)))
    {
        memcpy(&_bootSector, buffer, sizeof(FS_BootSector));
        _pageSize = _bootSector.pageSize;
        _fatEntryCount = bytesToUint24(_bootSector.fileCount);
    }
}
