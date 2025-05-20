# MjolnFileSystem  

**Author:** Saurav Sajeev  
**Description:** A file system designed for EEPROM storage management using AT24C series chips.

---

## **Table of Contents**  
1. [Overview](#overview)  
2. [Supported EEPROM Models](#supported-eeprom-models)  
3. [Features](#features)  
4. [Installation](#installation)  
5. [Usage](#usage)  
6. [Command Set](#command-set)  
7. [API Reference](#api-reference)  
   - Initialization and Mounting  
   - File Operations  
   - File System Information  
   - Terminal Interaction  
8. [Notes](#notes)  
9. [Warnings](#warnings)  
10. [License](#license)  

---

## **Overview**  
`MjolnFileSystem` is a lightweight, structured file system built for AT24C-series EEPROM chips. It provides functionality for file storage, retrieval, deletion, and system monitoring while optimizing memory usage. Designed for embedded systems, it allows efficient data management in constrained memory environments.

---

## **Supported EEPROM Models**  
The file system supports various AT24C EEPROM types, each with different storage capacities:

```cpp
enum AT24CXType
{
    AT24C04 = 0x09,   // 4 KB EEPROM
    AT24C08 = 0x0A,   // 8 KB EEPROM
    AT24C16 = 0x0B,   // 16 KB EEPROM
    AT24C32 = 0x0C,   // 32 KB EEPROM
    AT24C64 = 0x0D,   // 64 KB EEPROM
    AT24C128 = 0x0E,  // 128 KB EEPROM
    AT24C256 = 0x0F,  // 256 KB EEPROM
    AT24C512 = 0x10   // 512 KB EEPROM
};
```

The correct EEPROM model must be selected when initializing the file system.

---

## **Features**  
- **File Management**: Create, read, write, delete, and list files stored in EEPROM.  
- **Storage Monitoring**: Retrieve storage usage information, including percentage and bytes used.  
- **System Control**: Print file system details, format EEPROM, and manage logs.  
- **EEPROM Formatting**: Two formatting options:  
  - `format()`: Resets the boot sector and erases all data.  
  - `cleanFormat()`: Erases all data without reflashing the boot sector.  
- **Serial Terminal Interaction**: Execute commands via the serial interface for real-time file system management.

---

## **Installation**  

### 1. Clone the Repository  
To use `MjolnFileSystem`, clone the repository and navigate into the directory:

```bash
git clone https://github.com/YOUR_USERNAME/MjolnFileSystem.git
cd MjolnFileSystem
```

### 2. Initialize and Mount the File System  
Before using any file operations, initialize the file system:

```cpp
MjolnFileSystem fs(AT24C32);  // Initialize with chosen EEPROM model
if (!fs.mount()) {
    fs.format();  // Format EEPROM if unrecognized
}
```

Ensure that the correct EEPROM type is passed as a parameter.

---

## **Usage**  

### Writing Data to a File  
```cpp
fs.writeFile("config", "settings123");
```

### Reading Data from a File  
```cpp
char buffer[256]; // Allocate buffer
uint32_t length = fs.readFile("config.txt", buffer);
Serial.println(buffer);
delete[] buffer; // Free memory after use
```

### Deleting a File  
```cpp
fs.deleteFile("config.txt");
```

### Listing Stored Files  
```cpp
fs.listFiles();
```

---

## **Command Set**  

The `terminal()` function supports various commands for file system interactions.  

| **Command**         | **Description**                                  | **Example**                  |
|---------------------|--------------------------------------------------|------------------------------|
| `mk <filename> <data>` | Create a file and write data into it            | `mk config.txt settings123`  |
| `rm <filename>`    | Delete a specified file                          | `rm config.txt`              |
| `ls`               | List all available files                          | `ls`                         |
| `read <filename>`  | Read a file and display its contents              | `read config.txt`            |
| `info`             | Display general file system information           | `info`                       |
| `delpart`          | Format EEPROM and erase all stored data           | `delpart`                    |
| `storeuse`         | Show storage usage percentage                     | `storeuse`                    |
| `storeusebytes`    | Show the total number of bytes currently used     | `storeusebytes`               |
| `exit`             | Exit the terminal session                         | `exit`                        |

---

## **API Reference**  

### Initialization and Mounting  

```cpp
MjolnFileSystem(AT24CXType eepromModel);
bool mount();
```
- Initializes the file system with the specified EEPROM model.  
- If the EEPROM is unrecognized, `format()` should be used before mounting.

### File Operations  

```cpp
bool writeFile(const char *filename, const char *data);
uint32_t readFile(const char *filename, char *buffer);
bool deleteFile(const char *filename);
void listFiles();
```
- `writeFile()`: Creates and writes data to a file.  
- `readFile()`: Reads contents from a file into a dynamically allocated buffer. Caller must free memory.  
- `deleteFile()`: Deletes a specified file.  
- `listFiles()`: Lists all available files.  

### File System Information  

```cpp
void printFileSystemInfo();
void printFileInfo(const char *filename);
bool format();
bool cleanFormat();
float getStorageUsage();
uint32_t getBytesUsed();
```
- `printFileSystemInfo()`: Displays details about the file system.  
- `printFileInfo()`: Shows metadata and size of a specific file.  
- `format()`: Erases all data and resets the boot sector.  
- `cleanFormat()`: Erases all data without modifying the boot sector.  
- `getStorageUsage()`: Returns storage usage as a percentage.  
- `getBytesUsed()`: Returns total bytes occupied.  

### Terminal Interaction  

```cpp
void terminal();
```
- Provides a serial-based interface for executing file system commands.  
- Supports file manipulation, system queries, and formatting operations.  
- Allows users to exit the terminal session using the `exit` command.  

---

## **Notes**  

- **Memory Management**:  
  - The `readFile()` function returns a pointer to allocated memory. The caller must free it after use.  
  - Example:
    ```cpp
    char* buffer = new char[256];
    uint32_t len = fs.readFile("config.txt", buffer);
    Serial.println(buffer);
    delete[] buffer;
    ```
  
- **File System Formatting (`delpart`)**:  
  - Formatting permanently erases all stored data.  
  - Ensure backups are taken before executing this command.  

- **Serial Communication Considerations**:  
  - The terminal feature requires serial communication for command input.  
  - Commands must be properly formatted to function correctly.  

---

## **Warnings**  

- **FAT Entry Size Limitation:**  
  - The current FAT entry size is **16 bytes**, which limits **file names to a maximum of 8 characters**.  
  - Ensure that file names follow this restriction to avoid errors.  

---

## **License**  

This project is licensed under the **MIT License**. You are free to modify, distribute, and use it in personal or commercial projects.  
