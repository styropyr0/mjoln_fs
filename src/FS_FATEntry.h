#ifndef FS_FATENTRY_H
#define FS_FATENTRY_H

#include <Arduino.h>
#include "MjolnConst.h"

struct FS_FATEntry
{
    uint8_t startAddr[MJOLN_FILE_SYSTEM_START_ADDR_SIZE]; // Start address of the file in EEPROM
    uint8_t size[MJOLN_FILE_SYSTEM_FILE_SIZE];            // Size of the file in bytes
    char filename[MJOLN_FILE_NAME_MAX_LENGTH];            // File name (null-terminated)
    uint8_t status;                                       // Status of the file (0: free, 1: used)
};

FS_FATEntry toFATEntry(uint8_t *buffer, size_t bufferSize)
{
    FS_FATEntry fatEntry;

    if (bufferSize < sizeof(FS_FATEntry))
    {
        return {};
    }

    memcpy(fatEntry.startAddr, buffer, MJOLN_FILE_SYSTEM_START_ADDR_SIZE);
    memcpy(fatEntry.size, &buffer[MJOLN_FILE_SYSTEM_START_ADDR_SIZE], MJOLN_FILE_SYSTEM_FILE_SIZE);
    memcpy(fatEntry.filename, &buffer[MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE], MJOLN_FILE_NAME_MAX_LENGTH - 1);

    fatEntry.filename[MJOLN_FILE_NAME_MAX_LENGTH - 1] = '\0';
    fatEntry.status = buffer[MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE + MJOLN_FILE_NAME_MAX_LENGTH - 1];

    return fatEntry;
}

uint8_t *toBytes(FS_FATEntry *fatEntry)
{
    uint8_t *buffer = (uint8_t *)malloc(sizeof(FS_FATEntry));

    if (!buffer)
        return NULL;

    memcpy(buffer, fatEntry->startAddr, MJOLN_FILE_SYSTEM_START_ADDR_SIZE);
    memcpy(&buffer[MJOLN_FILE_SYSTEM_START_ADDR_SIZE], fatEntry->size, MJOLN_FILE_SYSTEM_FILE_SIZE);
    memcpy(&buffer[MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE], fatEntry->filename, MJOLN_FILE_NAME_MAX_LENGTH - 1);

    buffer[MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE + MJOLN_FILE_NAME_MAX_LENGTH - 1] = fatEntry->status;

    return buffer;
}

#endif // FS_FATENTRY_H
       // This file defines the structure of the FAT entry in the Mjoln EEPROM File System.