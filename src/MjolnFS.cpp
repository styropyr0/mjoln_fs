#include "MjolnFS.h"

MjolnFileSystem::MjolnFileSystem(AT24CXType eepromModel)
    : _eepromType(eepromModel), _deviceAddress(MJOLN_STORAGE_DEVICE_ADDRESS), _eepromSize(0), _pageSize(0), _fatEntries(nullptr), _fatEntryCount(0)
{
}

FS_BootSector MjolnFileSystem::readBootSector()
{
    FS_BootSector bootSector;
    uint8_t buffer[sizeof(FS_BootSector)];
    eepromReadBytes(MJOLN_STORAGE_DEVICE_ADDRESS, 0, getAddressSize(), buffer, sizeof(FS_BootSector), getPageSize());
    bootSector = toBootSector(buffer);
    return bootSector;
}

void MjolnFileSystem::writeBootSector(const FS_BootSector &bootSector)
{
    uint8_t *buffer = bootSectorToBytes(&bootSector);
    printLogs("Writing boot sector...\n");
    eepromWriteBytes(MJOLN_STORAGE_DEVICE_ADDRESS, 0, getAddressSize(), buffer, sizeof(FS_BootSector), getPageSize());
    printLogs("Boot sector written successfully.\n");
    printLogs("Boot sector data: ");

    for (size_t i = 0; i < sizeof(FS_BootSector); i++)
        printLogs(String(buffer[i], HEX) + " ");

    printLogs("\n");
    free(buffer);
}

FS_FATEntry MjolnFileSystem::readFATEntry(uint16_t index)
{
    FS_FATEntry fatEntry;
    uint8_t buffer[sizeof(FS_FATEntry)];
    eepromReadBytes(MJOLN_STORAGE_DEVICE_ADDRESS, sizeof(FS_BootSector) + index * sizeof(FS_FATEntry), getAddressSize(), buffer, sizeof(FS_FATEntry), getPageSize());
    fatEntry = toFATEntry(buffer, sizeof(buffer));
    return fatEntry;
}

bool MjolnFileSystem::writeFATEntry(uint16_t index, const FS_FATEntry &entry)
{
    uint8_t *buffer = fatToBytes(&entry);
    if (eepromWriteBytes(MJOLN_STORAGE_DEVICE_ADDRESS, sizeof(FS_BootSector) + index * sizeof(FS_FATEntry), getAddressSize(), buffer, sizeof(FS_FATEntry), getPageSize()))
    {
        free(buffer);
        printLogs("FAT entry written successfully.\n");
        return true;
    }
    else
    {
        free(buffer);
        printLogs("Failed to write FAT entry.\n");
        return false;
    }
}

bool MjolnFileSystem::begin()
{
    Wire.begin();
    _bootSector = readBootSector();
    if (verifyBootSector(&_bootSector))
    {
        uint32_t lastDataAddr = _bootSector.lastDataAddr[0] | (_bootSector.lastDataAddr[1] << 8) | (_bootSector.lastDataAddr[2] << 16);
        printLogs("\nMjoln File System\n-----------------\n");
        printLogs("EEPROM type: " + String(_eepromType) + "\n");
        printLogs("Valid file system signature.\n");
        printLogs("File system version: " + String(_bootSector.version) + "\n");
        printLogs("File system signature: " + String(_bootSector.signature) + "\n");
        printLogs("Last data address: " + String(lastDataAddr) + "\n");
        printLogs("File count: " + String(_bootSector.fileCount[0] | _bootSector.fileCount[1] << 8) + "\n");
        printLogs("\nSTORAGE USAGE\n-------------\n");
        printLogs(String(((lastDataAddr - getReservedSize()) * 100) / (pow(2, (uint8_t)_eepromType))) + "\% used from available space.\n");
        printLogs("Total: " + String((uint8_t)pow(2, (uint8_t)_eepromType)) + " bytes, " + String(getReservedSize()) + " bytes reserved by file system.\n");
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
    uint16_t reservedSize = getReservedSize();
    _bootSector.lastDataAddr[0] = reservedSize & 0xFF;
    _bootSector.lastDataAddr[1] = (reservedSize >> 8) & 0xFF;
    _bootSector.lastDataAddr[2] = (reservedSize >> 16) & 0xFF;
    _bootSector.fileCount[0] = 0;
    _bootSector.fileCount[1] = 0;

    writeBootSector(_bootSector);

    return true;
}

bool MjolnFileSystem::writeFile(const char *filename, const char *data)
{
    uint32_t length = strlen(data);
    FS_FATEntry fatEntry;
    fatEntry.status = 1;
    memcpy(fatEntry.filename, filename, MJOLN_FILE_NAME_MAX_LENGTH);
    fatEntry.size[0] = length & 0xFF;
    fatEntry.size[1] = (length >> 8) & 0xFF;
    fatEntry.size[2] = (length >> 16) & 0xFF;
    memcpy(fatEntry.startAddr, _bootSector.lastDataAddr, sizeof(fatEntry.startAddr));
    printLogs("Writing file...\n");
    uint32_t startAddr = _bootSector.lastDataAddr[0] | (_bootSector.lastDataAddr[1] << 8) | (_bootSector.lastDataAddr[2] << 16);

    if (writeFATEntry(_fatEntryCount + 1, fatEntry))
    {
        if (eepromWriteBytes(MJOLN_STORAGE_DEVICE_ADDRESS, startAddr, getAddressSize(), (const uint8_t *)data, length, getPageSize()))
        {
            _bootSector.lastDataAddr[0] = (startAddr + length) & 0xFF;
            _bootSector.lastDataAddr[1] = ((startAddr + length) >> 8) & 0xFF;
            _bootSector.lastDataAddr[2] = ((startAddr + length) >> 16) & 0xFF;
            _bootSector.fileCount[0] += 1;
            _bootSector.fileCount[1] += (_bootSector.fileCount[0] >> 8) & 0xFF;
            writeBootSector(_bootSector);
        }
        else
        {
            printLogs("Failed to write file data.\n");
            return false;
        }
        _fatEntryCount++;
    }

    if (logEnabled)
    {
        printLogs("\nFILE WRITE LOGS\n");
        printLogs("----------------\n");
        printLogs("File written successfully.\n");
        printLogs("File name: " + String(fatEntry.filename) + "\n");
        printLogs("File size: " + String(length) + "\n");
        printLogs("File start address: " + String(startAddr) + "\n");
        printLogs("File status: " + String(fatEntry.status) + "\n");
        printLogs("File data: ");
        for (size_t i = 0; i < length; i++)
            printLogs(String(data[i]));
        printLogs("\n\n");
    }

    return true;
}

char *MjolnFileSystem::readFile(const char *filename, char *buffer)
{
    for (uint16_t i = 1; i <= _fatEntryCount; i++)
    {
        FS_FATEntry fatEntry = readFATEntry(i);
        if (strcmp(fatEntry.filename, filename) == 0)
        {
            uint32_t length = fatEntry.size[0] | (fatEntry.size[1] << 8) | (fatEntry.size[2] << 16);
            uint32_t startAddr = fatEntry.startAddr[0] | (fatEntry.startAddr[1] << 8) | (fatEntry.startAddr[2] << 16);
            printLogs("Reading file...\n");
            eepromReadBytes(MJOLN_STORAGE_DEVICE_ADDRESS, startAddr, getAddressSize(), (uint8_t *)buffer, length, getPageSize());
            if (logEnabled)
            {
                printLogs("\nFILE READ LOGS\n");
                printLogs("----------------\n");
                printLogs("File read successfully.\n");
                printLogs("File name: " + String(fatEntry.filename) + "\n");
                printLogs("File size: " + String(length) + "\n");
                printLogs("File start address: " + String(startAddr) + "\n");
                printLogs("File status: " + String(fatEntry.status) + "\n");
                printLogs("File data: ");
                for (size_t i = 0; i < length; i++)
                    printLogs(String(buffer[i]));
                printLogs("\n\n");
            }
            return (char *)buffer;
        }
    }
    printLogs("File not found.\n");
    return nullptr;
}

void MjolnFileSystem::showLogs(bool show)
{
    logEnabled = show;
}

uint8_t MjolnFileSystem::getPageSize()
{
    switch (_eepromType)
    {
    case AT24C04:
    case AT24C08:
    case AT24C16:
        return 16;
    case AT24C32:
    case AT24C64:
        return 32;
    case AT24C128:
    case AT24C256:
        return 64;
    case AT24C512:
        return 128;
    default:
        return 0;
    }
}

uint16_t MjolnFileSystem::getUsableSize()
{
    return _eepromSize - sizeof(FS_BootSector) - (_fatEntryCount * sizeof(FS_FATEntry));
}

uint16_t MjolnFileSystem::getReservedSize()
{
    switch (_eepromType)
    {
    case AT24C04:
        return 136;
    case AT24C08:
        return 256;
    case AT24C16:
        return 496;
    case AT24C32:
        return 916;
    case AT24C64:
        return 1816;
    case AT24C128:
        return 3016;
    case AT24C256:
        return 4216;
    case AT24C512:
        return 6016;
    default:
        return 0;
    }
}

AT24CX_ADDR_SIZE MjolnFileSystem::getAddressSize()
{
    return (_eepromType == AT24C04 || _eepromType == AT24C08 || _eepromType == AT24C16) ? AT24CX_8Bit : AT24CX_16Bit;
}