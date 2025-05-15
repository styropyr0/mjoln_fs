#include "FileSystemManager.h"

bool eepromReadBytes(uint8_t eepromAddr, uint32_t storeAddr, uint8_t *buffer, uint16_t length)
{
    Wire.beginTransmission(eepromAddr);
    Wire.write((storeAddr >> 8) & 0xFF);
    Wire.write(storeAddr & 0xFF);
    Wire.endTransmission();
    Wire.requestFrom((int)eepromAddr, (int)length);
    for (uint16_t i = 0; i < length && Wire.available(); i++)
        buffer[i] = Wire.read();
    return Wire.available() == 0;
}

bool eepromWriteBytes(uint8_t eepromAddr, uint32_t storeAddr, const uint8_t *data, uint16_t length)
{
    Wire.beginTransmission(eepromAddr);
    Wire.write((storeAddr >> 8) & 0xFF);
    Wire.write(storeAddr & 0xFF);
    for (uint16_t i = 0; i < length; i++)
        Wire.write(data[i]);
    return Wire.endTransmission() == 0;
}

bool deletePartition(uint8_t eepromAddr, uint16_t length)
{
    printLogs("Deleting partition...\n\n|--------------------|\n ");
    uint8_t progress = 0;
    for (uint16_t addr = 0; addr < length - 64; addr += 64)
    {
        Wire.beginTransmission(eepromAddr);
        Wire.write((addr >> 8) & 0xFF);
        Wire.write(addr & 0xFF);
        for (uint16_t i = 0; i < 64; i++)
        {
            Wire.write(0xFF);
        }
        yield();
        if ((addr * 100) / length > progress)
        {
            progress = (addr * 100) / length;
            if (progress % 5 == 0)
                printLogs("=");
        }
        if (Wire.endTransmission() != 0)
            return false;
        delay(5);
    }
    printLogs("=\n\n");
    printLogs("Partition deleted successfully.\n");
    return true;
}
