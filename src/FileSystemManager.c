#include "FileSystemManager.h"
#include <Wire.h>
#include <Arduino.h>
#include "MjolnConst.h"

uint8_t eepromReadBytes(uint8_t eepromAddr, uint32_t storeAddr, uint8_t *buffer, uint16_t length)
{
    Wire.beginTransmission(eepromAddr);
    Wire.write((storeAddr >> 8) & 0xFF);
    Wire.write(storeAddr & 0xFF);
    Wire.endTransmission();
    Wire.requestFrom(eepromAddr, length);
    for (uint16_t i = 0; i < length && Wire.available(); i++)
    {
        buffer[i] = Wire.read();
    }
    return length;
}

bool eepromWriteBytes(uint8_t eepromAddr, uint32_t storeAddr, const uint8_t *data, uint16_t length)
{
    Wire.beginTransmission(eepromAddr);
    Wire.write((storeAddr >> 8) & 0xFF);
    Wire.write(storeAddr & 0xFF);
    for (uint16_t i = 0; i < length; i++)
    {
        Wire.write(data[i]);
    }
    return Wire.endTransmission() == 0;
}