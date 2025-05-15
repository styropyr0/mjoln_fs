#ifndef MJOLN_CONST_H
#define MJOLN_CONST_H

#include <Arduino.h>

/**
 * @brief Mjoln EEPROM File System Constants
 * @note This file contains constants used in the Mjoln EEPROM File System.
 * @note The constants are used to define the size of the EEPROM, page size, and other parameters.
 * @note The constants are used in the Mjoln EEPROM File System implementation.
 */

#define MJOLN_FILE_SYSTEM_VERSION 1              // Version of the Mjoln EEPROM File System
#define MJOLN_SIGNATURE 0x1DBD34                 // Signature to identify the file system
#define MJOLN_FILE_NAME_MAX_LENGTH 9             // Maximum length of the file name
#define MJOLN_FILE_SYSTEM_SIGNATURE_SIZE 8       // Length of the file system signature
#define MJOLN_FILE_SYSTEM_PAGE_SIZE 64           // Size of a page in EEPROM
#define MJOLN_FILE_SYSTEM_FAT_SIZE 16            // Size of the FAT in pages
#define MJOLN_FILE_SYSTEM_FAT_COUNT 2            // Number of FAT copies
#define MJOLN_FILE_SYSTEM_RESERVED_SIZE 2        // Size of the reserved area in the file system
#define MJOLN_FILE_SYSTEM_MAX_FILES 16           // Maximum number of files in the file system
#define MJOLN_FILE_SYSTEM_START_ADDR_SIZE 0x03   // Start address of the file system in EEPROM
#define MJOLN_FILE_SYSTEM_FILE_SIZE 0x03         // Size of the file in bytes
#define MJOLN_FILE_SYSTEM_FILE_COUNT_LENGTH 0x02 // Maximum size of a file in bytes

#endif // MJOLN_CONST_H