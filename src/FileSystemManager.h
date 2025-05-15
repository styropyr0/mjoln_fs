#ifndef FILESYSTEM_MANAGER_H
#define FILESYSTEM_MANAGER_H

#ifdef __cplusplus

#include <Arduino.h>
#include <Wire.h>
#include "Logger.h"
#include "MjolnConst.h"

/**
 * @brief Reads a specified number of bytes from the EEPROM.
 * @param eepromAddr The I2C address of the EEPROM.
 * @param storeAddr The starting address in the EEPROM to read from.
 * @param buffer Pointer to the buffer where the read data will be stored.
 * @param length The number of bytes to read.
 */
bool eepromReadBytes(uint8_t eepromAddr, uint32_t storeAddr, uint8_t *buffer, uint16_t length);

/**
 * @brief Writes a specified number of bytes to the EEPROM.
 * @param eepromAddr The I2C address of the EEPROM.
 * @param storeAddr The starting address in the EEPROM to write to.
 * @param data Pointer to the data to be written.
 * @param length The number of bytes to write.
 * @return true if the write operation was successful, false otherwise.
 */
bool eepromWriteBytes(uint8_t eepromAddr, uint32_t storeAddr, const uint8_t *data, uint16_t length);

bool deletePartition(uint8_t eepromAddr, uint16_t length);

#endif // __cplusplus
#endif // FILESYSTEM_MANAGER_H