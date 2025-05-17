#include "FS_BootSector.h"

FS_BootSector toBootSector(uint8_t *buffer)
{
    FS_BootSector bootSector;

    memcpy(bootSector.signature, buffer, MJOLN_FILE_SYSTEM_SIGNATURE_SIZE);
    bootSector.version = buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE];
    memcpy(bootSector.lastDataAddr, &buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 1], 3);
    bootSector.pageSize = buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 4];
    memcpy(bootSector.fileCount, &buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 5], MJOLN_FILE_SYSTEM_FILE_COUNT_LENGTH);
    bootSector.signature[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE - 1] = '\0';
    memcpy(bootSector.reserved, &buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 7], 1);

    return bootSector;
}

uint8_t *bootSectorToBytes(const FS_BootSector *bootSector)
{
    uint8_t *buffer = (uint8_t *)malloc(sizeof(FS_BootSector));

    if (!buffer)
        return NULL;

    memcpy(buffer, bootSector->signature, MJOLN_FILE_SYSTEM_SIGNATURE_SIZE);
    buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE] = bootSector->version;
    memcpy(&buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 1], bootSector->lastDataAddr, 3);
    buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 4] = bootSector->pageSize;
    memcpy(&buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 5], bootSector->fileCount, MJOLN_FILE_SYSTEM_FILE_COUNT_LENGTH);
    memcpy(&buffer[MJOLN_FILE_SYSTEM_SIGNATURE_SIZE + 7], bootSector->reserved, 1);

    return buffer;
}

bool verifyBootSector(FS_BootSector *bootSector)
{
    if (memcmp(bootSector->signature, MJOLN_SIGNATURE, MJOLN_FILE_SYSTEM_SIGNATURE_SIZE) != 0)
        return false;

    if (bootSector->version != MJOLN_FILE_SYSTEM_VERSION)
        return false;

    return true;
}