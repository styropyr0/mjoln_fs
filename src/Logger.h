#pragma once

#ifdef __cplusplus

#include <Arduino.h>

extern bool logEnabled; // Flag to control logging

void printLogs(String message);

#endif // __cplusplus
       // This file defines the logging functionality for the Mjoln EEPROM File System.