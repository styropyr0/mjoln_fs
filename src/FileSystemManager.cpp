#include "FileSystemManager.h"

bool eepromReadBytes(uint8_t eepromAddr, uint32_t storeAddr, AT24CX_ADDR_SIZE addressSize, uint8_t *buffer, uint16_t length, uint8_t pageSize)
{
    uint16_t bytesRead = 0;
    while (length > 0)
    {
        uint16_t remainingPageSize = min(length, (uint16_t)(pageSize - (storeAddr % pageSize)));
        Wire.beginTransmission(eepromAddr);
        if (addressSize == AT24CX_16Bit)
            Wire.write((storeAddr >> 8) & 0xFF);
        Wire.write(storeAddr & 0xFF);
        Wire.endTransmission();
        Wire.requestFrom((int)eepromAddr, (int)remainingPageSize);
        for (uint16_t i = 0; i < remainingPageSize && Wire.available(); i++)
            buffer[bytesRead++] = Wire.read();
        length -= remainingPageSize;
        storeAddr += remainingPageSize;
    }
    return Wire.available() == 0;
}

bool eepromWriteBytes(uint8_t eepromAddr, uint32_t storeAddr, AT24CX_ADDR_SIZE addressSize, const uint8_t *data, uint16_t length, uint8_t pageSize)
{
    uint16_t bytesWrote = 0;
    while (length > 0)
    {
        uint16_t remainingPageSize = min(length, (uint16_t)(pageSize - (storeAddr % pageSize)));
        delay(5);
        Wire.beginTransmission(eepromAddr);
        if (addressSize == AT24CX_16Bit)
            Wire.write((storeAddr >> 8) & 0xFF);
        Wire.write(storeAddr & 0xFF);
        for (uint16_t i = 0; i < remainingPageSize; i++)
            Wire.write(data[bytesWrote++]);
        length -= remainingPageSize;
        storeAddr += remainingPageSize;
        if (Wire.endTransmission() != 0)
            return 0;
    }
    return true;
}

void readMemoryBlock(uint16_t start, uint16_t end)
{
    for (uint16_t addr = start; addr < end; addr++)
    {
        printLogs("Addr " + String(addr) + " - " + String(addr + 128) + ": ");
        Wire.beginTransmission(0x50);
        Wire.write((addr >> 8) & 0xFF);
        Wire.write(addr & 0xFF);
        Wire.endTransmission();
        Wire.requestFrom(0x50, end - start);
        for (uint16_t i = 0; i < 128; i++)
        {
            printLogs("0x" + String(Wire.read(), HEX) + ", ");
        }
        printLogs("\n");
        addr += 128;
    }
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
