#include "MjolnFS.h"

MjolnFileSystem::MjolnFileSystem(AT24CXType eepromModel)
    : _eepromType(eepromModel), _deviceAddress(MJOLN_STORAGE_DEVICE_ADDRESS), _eepromSize(0), _pageSize(0), _fatEntryCount(0), signature(MJOLN_SIGNATURE)
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

bool MjolnFileSystem::writeBootSector(const FS_BootSector &bootSector)
{
    uint8_t *buffer = bootSectorToBytes(&bootSector);
    printLogs("Writing boot sector...\n");
    if (eepromWriteBytes(MJOLN_STORAGE_DEVICE_ADDRESS, 0, getAddressSize(), buffer, sizeof(FS_BootSector), getPageSize()))
    {
        printLogs("Boot sector written successfully.\n");
        printLogs("Boot sector data: ");

        for (size_t i = 0; i < sizeof(FS_BootSector); i++)
            printLogs(String(buffer[i], HEX) + " ");

        printLogs("\n");
        free(buffer);
        return true;
    }

    free(buffer);
    return false;
}

FS_FATEntry MjolnFileSystem::readFATEntry(uint16_t index)
{
    FS_FATEntry fatEntry;
    uint8_t buffer[sizeof(FS_FATEntry)];
    eepromReadBytes(MJOLN_STORAGE_DEVICE_ADDRESS, sizeof(FS_BootSector) + (index * sizeof(FS_FATEntry)), getAddressSize(), buffer, sizeof(FS_FATEntry), getPageSize());
    fatEntry = toFATEntry(buffer, sizeof(buffer));
    return fatEntry;
}

bool MjolnFileSystem::writeFATEntry(uint16_t index, const FS_FATEntry &entry)
{
    uint8_t *buffer = fatToBytes(&entry);
    if (eepromWriteBytes(MJOLN_STORAGE_DEVICE_ADDRESS, sizeof(FS_BootSector) + (index * sizeof(FS_FATEntry)), getAddressSize(), buffer, sizeof(FS_FATEntry), getPageSize()))
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

bool MjolnFileSystem::updateFATEntry(uint16_t index, const FS_FATEntry &entry)
{
    uint8_t *buffer = fatToBytes(&entry);
    if (eepromWriteBytes(MJOLN_STORAGE_DEVICE_ADDRESS, sizeof(FS_BootSector) + index * sizeof(FS_FATEntry), getAddressSize(), buffer, sizeof(FS_FATEntry), getPageSize()))
    {
        free(buffer);
        return true;
    }
    else
    {
        free(buffer);
        return false;
    }
}

bool MjolnFileSystem::mount()
{
    Wire.begin();
    _bootSector = readBootSector();
    if (verifyBootSector(&_bootSector))
    {
        _pageSize = _bootSector.pageSize;
        _fatEntryCount = _bootSector.fileCount[0] | (_bootSector.fileCount[1] << 8);
        uint32_t lastDataAddr = _bootSector.lastDataAddr[0] | (_bootSector.lastDataAddr[1] << 8) | (_bootSector.lastDataAddr[2] << 16);

        printLogs("Mounting file system...\n");
        runInitialIndexingAndStore();
        printLogs("File system mounted.");

        printLogs("\nMjoln File System\n-----------------\n");
        printLogs("EEPROM type: " + String(_eepromType) + "\n");
        printLogs("Valid file system signature.\n");
        printLogs("File system version: " + String(_bootSector.version) + "\n");
        printLogs("File system signature: " + String(_bootSector.signature) + "\n");
        printLogs("Last data address: " + String(lastDataAddr) + "\n");
        printLogs("File count: " + String(_fatEntryCount - (uint16_t)_bootSector.deleted) + "\n\n");
        isInit = true;
        getBytesUsed();
    }
    else
    {
        printLogs("Invalid file system signature.\n");
        return false;
    }
    return true;
}

bool MjolnFileSystem::format()
{
    printLogs("Formatting file system...\n");
    if (!cleanFormat())
        return false;

    _bootSector.version = MJOLN_FILE_SYSTEM_VERSION;
    memcpy(_bootSector.signature, signature, MJOLN_FILE_SYSTEM_SIGNATURE_SIZE);
    _bootSector.pageSize = MJOLN_FILE_SYSTEM_PAGE_SIZE;
    uint16_t reservedSize = getReservedSize();
    _bootSector.lastDataAddr[0] = reservedSize & 0xFF;
    _bootSector.lastDataAddr[1] = (reservedSize >> 8) & 0xFF;
    _bootSector.lastDataAddr[2] = (reservedSize >> 16) & 0xFF;
    _bootSector.fileCount[0] = 0;
    _bootSector.fileCount[1] = 0;
    _bootSector.deleted = 0;
    _bootSector.bytesInUse = 0;
    _fatEntryCount = 0;

    if (!writeBootSector(_bootSector))
    {
        _bootSector = readBootSector();
        return false;
    }
    isInit = true;
    return true;
}

bool MjolnFileSystem::cleanFormat()
{
    if (!Wire.available())
        Wire.begin();

    if (!deletePartition(MJOLN_STORAGE_DEVICE_ADDRESS, (uint16_t)(pow(2, (uint8_t)_eepromType) - 1)))
    {
        printLogs("Failed to delete partition.\n");
        return false;
    }
    return true;
}

bool MjolnFileSystem::updateFile(const char *filename, const char *data)
{
    if (!isFileSystemInitialized())
        return false;

    uint16_t index = checkFileExistence(filename);

    if (index != MJOLN_FILE_NOT_FOUND)
    {
        FS_FATEntry fatEntry;
        uint32_t length = strlen(data);
        fatEntry.size[0] = length & 0xFF;
        fatEntry.size[1] = (length >> 8) & 0xFF;
        fatEntry.size[2] = (length >> 16) & 0xFF;
        uint32_t startAddr = tempFatEntry.startAddr[0] | (tempFatEntry.startAddr[1] << 8) | (tempFatEntry.startAddr[2] << 16);
        printLogs("Updating file...\n");
        if (length <= (tempFatEntry.size[0] | (tempFatEntry.size[1] << 8) | (tempFatEntry.size[2] << 16)))
        {
            uint32_t dataLength = tempFatEntry.size[0] | (tempFatEntry.size[1] << 8) | (tempFatEntry.size[2] << 16);
            eepromDeleteMemoryRange(MJOLN_STORAGE_DEVICE_ADDRESS, startAddr, getAddressSize(), dataLength, getPageSize());
            if (!eepromWriteBytes(MJOLN_STORAGE_DEVICE_ADDRESS, startAddr, getAddressSize(), (const uint8_t *)data, length + 1, getPageSize()))
                printLogs("Failed to update the file data.\n");
            else
                updateFATEntry(index, fatEntry);
        }
        else
        {
            bool logState = logEnabled;
            if (logEnabled)
                showLogs(false);
            bool res = deleteFile(filename) && writeFile(filename, data);
            if (logEnabled)
                showLogs(logEnabled);
            if (!res)
            {
                printLogs("Failed to update the file data.\n");
                return false;
            }
        }

        if (logEnabled)
        {
            printLogs("\nFILE UPDATE LOGS\n");
            printLogs("----------------\n");
            printLogs("File updated successfully.\n");
            printLogs("File name: " + String(fatEntry.filename) + "\n");
            printLogs("File size: " + String(length) + " bytes\n");
            printLogs("File start address: " + String(startAddr) + "\n");
            printLogs("File status: " + String(fatEntry.status) + "\n");
            printLogs("File data: ");
            for (size_t i = 0; i < length; i++)
                printLogs(String(data[i]));
            printLogs("\n\n");
        }

        return true;
    }
    printLogs("File not found!\n");
    return false;
}

bool MjolnFileSystem::writeFile(const char *filename, const char *data)
{
    if (!isFileSystemInitialized())
        return false;

    if (checkFileExistence(filename) == MJOLN_FILE_NOT_FOUND)
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
                _bootSector.bytesInUse += length;
                writeBootSector(_bootSector);
            }
            else
            {
                printLogs("Failed to write file data.\n");
                return false;
            }
            _fatEntryCount++;
            if (_fatEntryCount > 1)
                fileLookupList.concat(",");
            fileLookupList.concat(fatEntry.filename);
        }

        if (logEnabled)
        {
            printLogs("\nFILE WRITE LOGS\n");
            printLogs("----------------\n");
            printLogs("File written successfully.\n");
            printLogs("File name: " + String(fatEntry.filename) + "\n");
            printLogs("File size: " + String(length) + " bytes\n");
            printLogs("File start address: " + String(startAddr) + "\n");
            printLogs("File status: " + String(fatEntry.status) + "\n");
            printLogs("File data: ");
            for (size_t i = 0; i < length; i++)
                printLogs(String(data[i]));
            printLogs("\n\n");
        }

        return true;
    }
    printLogs("Couldn't write this file. File already exists!\n");
    return false;
}

uint32_t MjolnFileSystem::readFile(const char *filename, char *buffer)
{
    if (!isFileSystemInitialized())
        return 0;

    uint16_t i = checkFileExistence(filename);
    if (i != MJOLN_FILE_NOT_FOUND)
    {
        uint32_t length = tempFatEntry.size[0] | (tempFatEntry.size[1] << 8) | (tempFatEntry.size[2] << 16);
        uint32_t startAddr = tempFatEntry.startAddr[0] | (tempFatEntry.startAddr[1] << 8) | (tempFatEntry.startAddr[2] << 16);
        uint32_t totalLength = length;
        printLogs("Reading file...\n");
        while (true)
        {
            eepromReadBytes(MJOLN_STORAGE_DEVICE_ADDRESS, startAddr, getAddressSize(), (uint8_t *)buffer, length, getPageSize());
            if (tempFatEntry.link != MJOLN_FILE_NOT_FOUND)
            {
                uint16_t nextIndex = tempFatEntry.link;
                tempFatEntry = readFATEntry(nextIndex);
                startAddr = tempFatEntry.startAddr[0] | (tempFatEntry.startAddr[1] << 8) | (tempFatEntry.startAddr[2] << 16);
                length = tempFatEntry.size[0] | (tempFatEntry.size[1] << 8) | (tempFatEntry.size[2] << 16);
                buffer += length;
                totalLength += length;
            }
            else
                break;

            buffer[totalLength] = '\0';
            if (logEnabled)
            {
                printLogs("\nFILE READ LOGS\n");
                printLogs("----------------\n");
                printLogs("File read successfully.\n");
                printLogs("File name: " + String(tempFatEntry.filename) + "\n");
                printLogs("File size: " + String(totalLength) + " bytes\n");
                printLogs("File start address: " + String(startAddr) + "\n");
                printLogs("File data: ");
                for (size_t i = 0; i < totalLength; i++)
                    printLogs(String(buffer[i]));
                printLogs("\n\n");
            }
            return totalLength;
        }
        printLogs("File not found.\n");
        return 0;
    }

    bool MjolnFileSystem::deleteFile(const char *filename)
    {
        if (!isFileSystemInitialized())
            return false;

        uint16_t i = checkFileExistence(filename);
        if (i != MJOLN_FILE_NOT_FOUND)
        {
            tempFatEntry.status = 0;
            if (updateFATEntry(i, tempFatEntry))
            {
                uint32_t length = tempFatEntry.size[0] | (tempFatEntry.size[1] << 8) | (tempFatEntry.size[2] << 16);
                uint32_t startAddr = tempFatEntry.startAddr[0] | (tempFatEntry.startAddr[1] << 8) | (tempFatEntry.startAddr[2] << 16);
                printLogs("Deleting file...\n");
                if (eepromDeleteMemoryRange(MJOLN_STORAGE_DEVICE_ADDRESS, startAddr, getAddressSize(), length, getPageSize()))
                {
                    if ((_bootSector.lastDataAddr[0] | (_bootSector.lastDataAddr[1] << 8) | (_bootSector.lastDataAddr[2] << 16)) - length == startAddr)
                    {
                        _bootSector.lastDataAddr[0] = (startAddr - length) & 0xFF;
                        _bootSector.lastDataAddr[1] = ((startAddr - length) >> 8) & 0xFF;
                        _bootSector.lastDataAddr[2] = ((startAddr - length) >> 16) & 0xFF;
                    }
                    _bootSector.bytesInUse -= length;
                    _bootSector.deleted++;
                    deleteLinks(tempFatEntry.link);
                    writeBootSector(_bootSector);
                    delay(5);
                    _bootSector = readBootSector();
                }
                else
                {
                    printLogs("Failed to delete the file.\n");
                    tempFatEntry.status = 1;
                    updateFATEntry(i, tempFatEntry);
                    return false;
                }
                if (logEnabled)
                {
                    printLogs("\nFILE DELETE LOGS\n");
                    printLogs("----------------\n");
                    printLogs("File deleted successfully.\n");
                    printLogs("File name: " + String(tempFatEntry.filename) + "\n");
                    printLogs("File size: " + String(length) + " bytes\n");
                    printLogs("File start address: " + String(startAddr) + "\n");
                    printLogs("File status: DELETED\n\n");
                }
                return true;
            }
        }
        printLogs("File not found.\n");
        return false;
    }

    bool MjolnFileSystem::deleteLinks(uint32_t firstLinkIndex)
    {
        while (firstLinkIndex != MJOLN_FILE_NOT_FOUND)
        {
            FS_FATEntry linkEntry = readFATEntry(firstLinkIndex);
            if (linkEntry.status == MJOLN_FILE_SYSTEM_FAT_UNAVAILABLE)
            {
                printLogs("Link entry not found.\n");
                return false;
            }
            uint32_t temp = linkEntry.link;
            linkEntry.link = MJOLN_FILE_NOT_FOUND;
            linkEntry.status = MJOLN_FILE_SYSTEM_FAT_UNAVAILABLE;
            if (!updateFATEntry(firstLinkIndex, linkEntry))
            {
                printLogs("Failed to update link entry.\n");
                return false;
            }
            firstLinkIndex = temp;
        }
        return true;
    }

    uint16_t MjolnFileSystem::newLinkEntry(uint16_t length)
    {
        if (!isFileSystemInitialized())
            return MJOLN_FILE_NOT_FOUND;

        for (uint16_t i = 0; i < voidFATEntryCacheSize; i++)
        {
            if (length >= voidFATEntryCache[i])
                return voidFATEntryCache[i];
        }
        return MJOLN_FILE_NOT_FOUND;
    }

    void MjolnFileSystem::findAllVoidFATEntries()
    {
        if (!isFileSystemInitialized())
            return;

        for (uint16_t i = 1; i <= _fatEntryCount; i++)
        {
            tempFatEntry = readFATEntry(i);
            if (tempFatEntry.status == MJOLN_FILE_SYSTEM_FAT_UNAVAILABLE && tempFatEntry.link != MJOLN_FILE_NOT_FOUND)
                voidFATEntryCache[voidFATEntryCacheSize++] = i;
        }
    }

    uint16_t MjolnFileSystem::checkFileExistence(const char *filename)
    {
        return findFileFromCache(filename);
    }

    void MjolnFileSystem::listFiles()
    {
        if (!isFileSystemInitialized())
            return;

        bool logState = logEnabled;
        showLogs(true);

        printLogs("FILES LIST\nroot\\\n");
        for (uint16_t i = 1; i <= _fatEntryCount; i++)
        {
            tempFatEntry = readFATEntry(i);
            if (tempFatEntry.status == MJOLN_FILE_SYSTEM_FAT_UNAVAILABLE)
                continue;
            printLogs("     " + String(tempFatEntry.filename) + (i % 8 == 0 ? "\n" : ""));
        }
        printLogs("\n");

        showLogs(logState);
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

    float MjolnFileSystem::getStorageUsage()
    {
        if (!isFileSystemInitialized())
            return -1;

        printLogs("\nSTORAGE USAGE\n-------------\n");
        float usage = (_bootSector.bytesInUse * 100) / (pow(2, (uint8_t)_eepromType));
        printLogs(String(usage) + "\% used from available space.\n");
        printLogs("Total: " + String((uint32_t)pow(2, (uint8_t)_eepromType)) + " bytes, " + String(getReservedSize()) + " bytes reserved by file system.\n\n");
        return usage;
    }

    uint32_t MjolnFileSystem::getBytesUsed()
    {
        if (!isFileSystemInitialized())
            return 0;

        printLogs("\nSTORAGE USAGE\n-------------\n");
        printLogs(String(_bootSector.bytesInUse) + " bytes used from available space.\n");
        printLogs("Total: " + String((uint32_t)pow(2, (uint8_t)_eepromType)) + " bytes, " + String(getReservedSize()) + " bytes reserved by file system.\n\n");
        return _bootSector.bytesInUse;
    }

    void MjolnFileSystem::printFileInfo(const char *filename)
    {
        if (!isFileSystemInitialized())
            return;

        bool logState = logEnabled;
        showLogs(true);

        if (checkFileExistence(filename))
        {
            uint32_t length = tempFatEntry.size[0] | (tempFatEntry.size[1] << 8) | (tempFatEntry.size[2] << 16);
            uint32_t startAddr = tempFatEntry.startAddr[0] | (tempFatEntry.startAddr[1] << 8) | (tempFatEntry.startAddr[2] << 16);
            printLogs("\nFILE INFORMATION\n");
            printLogs("----------------\n");
            printLogs("File name: " + String(tempFatEntry.filename) + "\n");
            printLogs("File size: " + String(length) + "\n");
            printLogs("File start address: " + String(startAddr) + "\n");
            printLogs("\n\n");
        }
        else
            printLogs("File not found!\n");

        showLogs(logState);
    }

    void MjolnFileSystem::printFileSystemInfo()
    {
        if (!isFileSystemInitialized())
            return;

        bool logState = logEnabled;
        showLogs(true);

        printLogs("\nMjoln File System\n-----------------\n");
        printLogs("EEPROM type: " + String(_eepromType) + "\n");
        printLogs("File system version: " + String(_bootSector.version) + "\n");
        printLogs("File system signature: " + String(_bootSector.signature) + "\n");
        printLogs("File count: " + String((_bootSector.fileCount[0] | _bootSector.fileCount[1] << 8) - (uint16_t)_bootSector.deleted) + "\n");
        printLogs("Total size: " + String((uint32_t)pow(2, (uint8_t)_eepromType)) + " Bytes\n");
        printLogs("Available size: " + String((uint32_t)pow(2, (uint8_t)_eepromType) - _bootSector.bytesInUse - getReservedSize()) + " Bytes\n");
        printLogs("Reserved size: " + String(getReservedSize()) + " Bytes\n");
        printLogs("Address size: " + String(getAddressSize() ? "16 bit\n" : "8 bit\n"));
        printLogs("Page size: " + String(getPageSize()) + " Bytes\n");
        printLogs("Storage use: " + String((_bootSector.bytesInUse * 100) / (pow(2, (uint8_t)_eepromType))) + "%\n\n");

        showLogs(logState);
    }

    void MjolnFileSystem::terminal()
    {
        if (!isFileSystemInitialized())
            return;

        String inputString = "";
        Serial.println("MJOLN FILE SYSTEM TERMINAL");
        Serial.print("\nmjolnFS@v1> ");
        showLogs(false);

        while (true)
        {
            if (Serial.available() > 0)
            {
                char inputChar = Serial.read();
                yield();

                if (inputChar == '\n')
                {
                    inputString.trim();

                    if (inputString.equals("exit"))
                    {
                        Serial.println("Exiting...");
                        break;
                    }
                    Serial.println(inputString);
                    processCommand(inputString);
                    inputString = "";
                    Serial.print("\nmjolnFS@v1> ");
                }
                else
                {
                    inputString += inputChar;
                }
            }
        }
        showLogs(true);
    }

    bool MjolnFileSystem::isFileSystemInitialized()
    {
        if (!isInit)
            printLogs("File system is not initialized. Possible reasons could be:\n-> Incompatible EEPROM\n-> Skipped mount method\n-> Connection failure to EEPROM\n\n");
        return isInit;
    }

    uint16_t MjolnFileSystem::findFileFromCache(const char *filename)
    {
        uint16_t pos = MJOLN_FILE_NOT_FOUND;

        if (_fatEntryCount > 0)
        {
            char filesBuffer[fileLookupList.length() + 1];
            strcpy(filesBuffer, fileLookupList.c_str());

            char *token = strtok(filesBuffer, ",");
            uint16_t index = loadBalancingState ? _fatEntryCount / 2 : 1;

            while (token != NULL)
            {
                if (strcmp(token, filename) == 0)
                {
                    tempFatEntry = readFATEntry(index);
                    if (tempFatEntry.status)
                    {
                        pos = index;
                        break;
                    }
                }
                token = strtok(NULL, ",");
                index++;
            }
        }

        if (loadBalancingState && pos == MJOLN_FILE_NOT_FOUND)
            for (uint16_t i = 1; i <= _fatEntryCount / 2; i++)
            {
                tempFatEntry = readFATEntry(i);
                if (tempFatEntry.status == MJOLN_FILE_SYSTEM_FAT_UNAVAILABLE)
                    continue;
                if (strcmp(tempFatEntry.filename, filename) == 0)
                    pos = i;
            }

        return pos;
    }

    void MjolnFileSystem::runInitialIndexingAndStore()
    {
        if (_fatEntryCount == 0)
            return;
        loadBalancingState = _fatEntryCount >= MJOLN_FILE_SYSTEM_CACHING_LIMIT;
        for (uint16_t i = (_fatEntryCount < MJOLN_FILE_SYSTEM_CACHING_LIMIT ? 1 : (_fatEntryCount / 2)); i <= _fatEntryCount; i++)
        {
            tempFatEntry = readFATEntry(i);
            fileLookupList += String(tempFatEntry.filename) + (i < _fatEntryCount ? "," : "");
        }
    }
