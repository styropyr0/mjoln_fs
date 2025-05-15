#include "MjolnFS.h"

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
    printLogs("Writing boot sector...\n");
    eepromWriteBytes(MJOLN_STORAGE_DEVICE_ADDRESS, 0, buffer, sizeof(FS_BootSector));
    printLogs("Boot sector written successfully.\n");
    printLogs("Boot sector data:");

    for (size_t i = 0; i < sizeof(FS_BootSector); i++)
        printLogs(String(buffer[i], HEX) + " ");

    printLogs("\n");
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
    {
        printLogs("Valid file system signature.\n");
        printLogs("File system version: " + String(_bootSector.version) + "\n");
        printLogs("File system signature: " + String(_bootSector.signature) + "\n");
    }
    else
    {
        printLogs("Invalid file system signature.\n");
        return false;
    }
    _pageSize = _bootSector.pageSize;
    _fatEntryCount = _bootSector.fileCount[0] | (_bootSector.fileCount[1] << 8);
    return true;
}

bool MjolnFileSystem::format()
{
    if (!deletePartition(MJOLN_STORAGE_DEVICE_ADDRESS, 65535))
    {
        printLogs("Failed to delete partition.\n");
        return false;
    }

    printLogs("Formatting file system...\n");
    _bootSector.version = MJOLN_FILE_SYSTEM_VERSION;
    memcpy(_bootSector.signature, signature, MJOLN_FILE_SYSTEM_SIGNATURE_SIZE);
    _bootSector.pageSize = MJOLN_FILE_SYSTEM_PAGE_SIZE;
    _bootSector.fileCount[0] = 0;
    _bootSector.fileCount[1] = 0;

    printLogs("Writing boot sector...\n");

    writeBootSector(_bootSector);

    printLogs("Boot sector written successfully.\n");

    return true;
}

void MjolnFileSystem::showLogs(bool show)
{
    logEnabled = show;
}
