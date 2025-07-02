#include "FS_FATEntry.h"

FS_FATEntry toFATEntry(uint8_t *buffer, size_t bufferSize)
{
    FS_FATEntry fatEntry;

    if (bufferSize < MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE + MJOLN_FILE_NAME_MAX_LENGTH + 1)
        return {};

    memcpy(fatEntry.startAddr, buffer, MJOLN_FILE_SYSTEM_START_ADDR_SIZE);
    memcpy(fatEntry.size, buffer + MJOLN_FILE_SYSTEM_START_ADDR_SIZE, MJOLN_FILE_SYSTEM_FILE_SIZE);
    memcpy(fatEntry.filename, buffer + MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE, MJOLN_FILE_NAME_MAX_LENGTH - 1);

    fatEntry.filename[MJOLN_FILE_NAME_MAX_LENGTH - 1] = '\0';
    memcpy(&fatEntry.link, buffer + MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE + MJOLN_FILE_NAME_MAX_LENGTH - 1, sizeof(fatEntry.link));
    fatEntry.status = buffer[MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE + MJOLN_FILE_NAME_MAX_LENGTH - 1 + sizeof(fatEntry.link)];

    return fatEntry;
}

uint8_t *fatToBytes(const FS_FATEntry *fatEntry)
{
    uint8_t *buffer = (uint8_t *)malloc(MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE + MJOLN_FILE_NAME_MAX_LENGTH + 1);

    if (!buffer)
        return NULL;

    memcpy(buffer, fatEntry->startAddr, MJOLN_FILE_SYSTEM_START_ADDR_SIZE);
    memcpy(buffer + MJOLN_FILE_SYSTEM_START_ADDR_SIZE, fatEntry->size, MJOLN_FILE_SYSTEM_FILE_SIZE);
    memcpy(buffer + MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE, fatEntry->filename, MJOLN_FILE_NAME_MAX_LENGTH - 1);
    buffer[MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE + MJOLN_FILE_NAME_MAX_LENGTH - 1] = '\0';
    memcpy(buffer + MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE + MJOLN_FILE_NAME_MAX_LENGTH - 1, &fatEntry->link, sizeof(fatEntry->link));
    buffer[MJOLN_FILE_SYSTEM_START_ADDR_SIZE + MJOLN_FILE_SYSTEM_FILE_SIZE + MJOLN_FILE_NAME_MAX_LENGTH - 1 + sizeof(fatEntry->link)] = fatEntry->status;

    return buffer;
}
