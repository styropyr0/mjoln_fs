#ifndef FS_BOOTSECTOR_H
#define FS_BOOTSECTOR_H

#include <Arduino.h>
#include "MjolnConst.h"

/**
 * @brief Mjoln EEPROM File System Boot Sector
 * @note This structure represents the boot sector of the Mjoln EEPROM File System.
 * @note It contains information about the file system version, signature, page size, and file count.
 */
struct FS_BootSector
{
    uint8_t signature[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE];    // Signature to identify the file system
    uint8_t version;                                        // Version of the file system
    uint8_t reserved[6];                                    // Reserved bytes
    uint8_t pageSize;                                       // Size of a page in EEPROM
    uint8_t fileCount[MJOLN_FILE_SYSTEM_FILE_COUNT_LENGTH]; // Number of files in the file system
};

FS_BootSector toBootSector(uint8_t *buffer)
{
    FS_BootSector bootSector;

    memcpy(bootSector.signature, buffer, MJOLN_FILE_SYSTEM_SIGNATURE_SIZE);
    bootSector.version = buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE];
    memcpy(bootSector.reserved, &buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 1], 6);
    bootSector.pageSize = buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 7];
    memcpy(bootSector.fileCount, &buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 8], MJOLN_FILE_SYSTEM_FILE_COUNT_LENGTH);
    bootSector.filename[MJOLN_FILE_NAME_MAX_LENGTH - 1] = '\0';

    return bootSector;
}

uint8_t *toBytes(FS_BootSector *bootSector)
{
    uint8_t *buffer = (uint8_t *)malloc(sizeof(FS_BootSector));

    if (!buffer)
        return NULL;

    memcpy(buffer, bootSector->signature, MJOLN_FILE_SYSTEM_SIGNATURE_SIZE);
    buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE] = bootSector->version;
    memcpy(&buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 1], bootSector->reserved, 6);
    buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 7] = bootSector->pageSize;
    memcpy(&buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 8], bootSector->fileCount, MJOLN_FILE_SYSTEM_FILE_COUNT_LENGTH);

    return buffer;
}

bool verifyBootSector(FS_BootSector *bootSector)
{
    for (int i = 0; i < MJOLN_FILE_SYSTEM_SIGNATURE_SIZE; i++)
    {
        if (bootSector->signature[i] != MJOLN_SIGNATURE)
            return false;
    }

    if (bootSector->version != MJOLN_FILE_SYSTEM_VERSION)
        return false;

    return true;
}

#endif // FS_BOOTSECTOR_H
       // This file defines the structure of the boot sector of the Mjoln EEPROM File System.