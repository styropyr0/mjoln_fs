#include "Logger.h"

bool logEnabled = true; // Flag to control logging

void printLogs(String message)
{
    if (logEnabled)
        Serial.print(message);
}