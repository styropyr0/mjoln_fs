#include "MjolnFS.h"

void MjolnFileSystem::processCommand(String command)
{
    if (command.startsWith("mk "))
    {
        String filename, data;
        extractArgs(command, filename, data);
        if (!filename.isEmpty() && !data.isEmpty())
        {
            if (writeFile(filename.c_str(), data.c_str()))
                Serial.println("File created.");
            else
                Serial.println("ERR: File already exists!");
        }
        else
            Serial.println("Usage: mk <filename> <data>");
    }
    else if (command.startsWith("update "))
    {
        String filename, data;
        extractArgs(command, filename, data);
        if (!filename.isEmpty() && !data.isEmpty())
        {
            if (updateFile(filename.c_str(), data.c_str()))
                Serial.println("File updated.");
            else
                Serial.println("ERR: File not found!");
        }
        else
            Serial.println("Usage: mk <filename> <data>");
    }
    else if (command.startsWith("rm "))
    {
        String filename = command.substring(3);
        filename.trim();
        if (!filename.isEmpty())
        {
            deleteFile(filename.c_str());
            Serial.println("File deleted.");
        }
        else
            Serial.println("Usage: rm <filename>");
    }
    else if (command.equals("ls"))
        listFiles();
    else if (command.startsWith("read "))
    {
        String filename = command.substring(5);
        filename.trim();

        if (!filename.isEmpty())
        {
            char *buffer = new char[256];
            if (buffer)
            {
                if (readFile(filename.c_str(), buffer))
                {
                    Serial.println(buffer);
                }
                else
                    Serial.println("ERR: File not found.");

                delete[] buffer;
            }
            else
            {
                Serial.println("ERR: Memory allocation failed.");
            }
        }
        else
            Serial.println("Usage: read <filename>");
    }
    else if (command.equals("info"))
        printFileSystemInfo();
    else if (command.equals("delpart"))
    {
        format();
        Serial.println("Partition deleted.");
    }
    else if (command.equals("storeuse"))
    {
        Serial.print("Storage Usage: ");
        Serial.print(getStorageUsage(), 2);
        Serial.println("%");
    }
    else if (command.equals("storeusebytes"))
    {
        Serial.print("Bytes Used: ");
        Serial.println(getBytesUsed());
    }
    else
        Serial.println("Unknown command.");
}

void MjolnFileSystem::extractArgs(String command, String &filename, String &data)
{
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);

    if (firstSpace != -1 && secondSpace != -1)
    {
        filename = command.substring(firstSpace + 1, secondSpace);
        filename.trim();
        data = command.substring(secondSpace + 1);
        data.trim();
    }
}
