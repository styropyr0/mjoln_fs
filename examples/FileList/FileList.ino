#include <MjolnFS.h>

MjolnFileSystem fs(AT24C32);

void setup()
{
    Serial.begin(9600);
    delay(5000);
    if (!fs.mount()) // If mounting fails, it means the file system is not found or invalid
        fs.format(); // Initially, the eeprom shall be formatted with MjolnFS File System's Boot sector
    fs.writeFile("file1", "Maupertuis set about generalising his earlier mathematical work");
    fs.writeFile("file2", ", proposing the principle of least action as a metaphysical principle that underlies");
    fs.writeFile("file3", "all the laws of mechanics. He also expanded into the biological realm, anonymously");
    fs.writeFile("lastFile", "publishing a book that was part popular science, part philosophy, and part erotica: Venus physique.");
    fs.listFiles();
}

void loop()
{
}