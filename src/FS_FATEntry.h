#ifndef FS_FATENTRY_H
#define FS_FATENTRY_H

#include <Arduino.h>
#include "MjolnConst.h"

#ifdef __cplusplus

/**
 * @brief Mjoln EEPROM File System FAT Entry
 * @note This structure represents a FAT entry in the Mjoln EEPROM File System.
 * @note It contains information about the start address, size, filename, and status of the file.
 */
struct FS_FATEntry
{
    uint8_t startAddr[MJOLN_FILE_SYSTEM_START_ADDR_SIZE]; // Start address of the file in EEPROM
    uint8_t size[MJOLN_FILE_SYSTEM_FILE_SIZE];            // Size of the file in bytes
    char filename[MJOLN_FILE_NAME_MAX_LENGTH];            // File name (null-terminated)
    uint8_t status;                                       // Status of the file (0: free, 1: used)
};

/**
 * @brief Converts a byte buffer to a FS_FATEntry structure.
 * @param buffer Pointer to the byte buffer.
 * @param bufferSize Size of the buffer.
 * @return FS_FATEntry structure.
 * @note The buffer must be at least sizeof(FS_FATEntry) bytes long.
 */
FS_FATEntry toFATEntry(uint8_t *buffer, size_t bufferSize);

/**
 * @brief Converts a FS_FATEntry structure to a byte buffer.
 * @param fatEntry Pointer to the FS_FATEntry structure.
 * @return Pointer to the byte buffer.
 * @note The caller is responsible for freeing the allocated memory.
 */
uint8_t *fatToBytes(const FS_FATEntry *fatEntry);

#endif //__cplusplus
#endif // FS_FATENTRY_H
       // This file defines the structure of the FAT entry in the Mjoln EEPROM File System.