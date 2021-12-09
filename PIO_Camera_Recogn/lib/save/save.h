#ifndef SAVE_H
#define SAVE_H

#include <EEPROM.h> // read and write from flash memory
#include "FS.h"               // SD Card ESP32
#include "SD_MMC.h"           // SD Card ESP32

#include "config.h"

// define the number of bytes you want to access
#define EEPROM_SIZE 3000

// The Indices of the EEPROM Values
#define EEPROM_IMAGE_COUNT 0
#define EEPROM_BRIMAP_SIZE 1
#define EEPROM_BRIMAP_DATA 2

uint8_t brimap[END_RESOLUTION][END_RESOLUTION][3];

bool saveImgToSd(const uint8_t *frame, size_t frame_len, String name);
bool saveImgToSdPPM(uint8_t frame[END_RESOLUTION][END_RESOLUTION][3], size_t frame_len, String name);
void countEEPROM();
void initSD();
void initEEPROM();
void loadBrimap();
void saveBrimap();

bool debug_sd_initialised = false;
bool debug_ee_initialised = false;

bool saveImgToSd(const uint8_t *frame, size_t frame_len, String name)
{
    int pictureNumber = EEPROM.read(EEPROM_IMAGE_COUNT);
    // Path where new picture will be saved in SD Card
    String path = "/debug/" + String(pictureNumber) + "_" + name + ".jpg";

    fs::FS &fs = SD_MMC;
    Serial.printf("Picture file path: %s\n", path.c_str());

    File file = fs.open(path.c_str(), FILE_WRITE);
    if (!file)
    {
        Serial.println("Failed to open file in writing mode");
        return false;
    }
    else
    {
        file.write(frame, frame_len); // payload (image), payload length
        Serial.printf("Saved file to path: %s\n", path.c_str());
    }
    file.close();

    return true;
}

bool saveImgToSdPPM(uint8_t frame[END_RESOLUTION][END_RESOLUTION][3], size_t frame_len, String name)
{
    int pictureNumber = EEPROM.read(0);

    // Path where new picture will be saved in SD Card
    String path = "/debug/" + String(pictureNumber) + "_" + name + ".ppm";

    fs::FS &fs = SD_MMC;
    Serial.printf("[PPM Mode] Picture file path: %s\n\r", path.c_str());

    File file = fs.open(path.c_str(), FILE_WRITE);
    if (!file)
    {
        Serial.println("[PPM Mode] Failed to open file in writing mode");
        return false;
    }
    else
    {
        file.printf("P6\n%d %d\n255\n", END_RESOLUTION, END_RESOLUTION);
        for (int j = 0; j < END_RESOLUTION; ++j)
        {
            for (int i = 0; i < END_RESOLUTION; ++i)
            {
                file.write(frame[j][i], 3); // payload (image), payload length
            }
        }
        Serial.printf("[PPM Mode] Saved file to path: %s\n", path.c_str());
    }
    file.close();
    return true;
}

void countEEPROM()
{
    initEEPROM();
    EEPROM.write(EEPROM_IMAGE_COUNT, EEPROM.read(EEPROM_IMAGE_COUNT) + 1);
    EEPROM.commit();
}

void initSD()
{
    initEEPROM();
    if (debug_sd_initialised)
    {
        SD_MMC.end();
    }
    debug_sd_initialised = true;
    // Serial.println("Initialising SD Card. Unplug the NeoPixel Jumper! 5s");
    // delay(5000);

    Serial.println("Starting SD Card");

    // pinMode(2, INPUT_PULLUP);
    // pinMode(4, INPUT_PULLUP);

    // pinMode(2, OUTPUT);
    // pinMode(4, OUTPUT);
    // pinMode(12, OUTPUT);
    // pinMode(13, OUTPUT);
    // pinMode(14, OUTPUT);
    // pinMode(15, OUTPUT);

    if (!SD_MMC.begin())
    {
        Serial.println("SD Card Mount Failed");
        debug_sd_initialised = false;
        return;
    }

    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE)
    {
        Serial.println("No SD Card attached");
        debug_sd_initialised = false;
        return;
    }

    Serial.print("SD_MMC Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\r\n", cardSize);
    Serial.println("Done!");
}

void initEEPROM()
{
    if (!debug_ee_initialised)
    {
        debug_ee_initialised = true;

        Serial.println("Initialise EEPROM");
        EEPROM.begin(EEPROM_SIZE);
        Serial.println("Done!");
    }
}

void loadBrimap()
{
    initEEPROM();
    Serial.println("Loading Brimap");
    int brimap_size = EEPROM.read(EEPROM_BRIMAP_SIZE);
    if (brimap_size != END_RESOLUTION)
    {
        Serial.println("Error loading Brimap: Different dimensions! Recalibrate NOW!");
    }
    EEPROM.get(EEPROM_BRIMAP_DATA, brimap);
}
void saveBrimap()
{
    initEEPROM();
    Serial.println("Saving Brimap");
    EEPROM.write(EEPROM_BRIMAP_SIZE, END_RESOLUTION);
    EEPROM.put(EEPROM_BRIMAP_DATA, brimap);
    EEPROM.commit();
}

#endif
