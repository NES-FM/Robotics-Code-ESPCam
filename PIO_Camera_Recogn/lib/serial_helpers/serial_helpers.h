#ifndef SERIAL_HELPERS_H
#define SERIAL_HELPERS_H

#include <Arduino.h>
#include "config.h"

void print_frame(uint8_t frame[END_RESOLUTION][END_RESOLUTION][3]);
void print_width(uint8_t *frame);
void serial_interface_tick();

/*
Help: This shows the different commands (commands are case sensitive)
    d: toggles Debug mode
    i: Initialises SD Card

    r: recalibrates: White image as brimap -> Substracting
    s: Saves the Current Brimap to SD

    W: Saves Original + Downsize to SD
    B: Saves Original + Downsize + Brimap to SD
    C: Saves Original + Downsize/Brimap/Color to SD

    b: Saves Downsize + Brimap to SD
    c: Saves Downsize/Brimap/Color to SD

    o: Saves Original to SD
    w: Saves Downsize to SD

    e: Resets EEPROM to 0
    p: prints whole content of eeprom

    h: Shows the help
*/
void serial_interface_tick()
{
    if (Serial.available())
    {
        char s = Serial.read();
        if (s == 'd')
        {
            debug_enabled = !debug_enabled;
            String den = debug_enabled ? "On" : "Off";
            Serial.println("Debug Toogled. Debug is now " + den);
        }
        else if (s == 'i')
        {
            initSD();
        }
        else if (s == 'W')
        {
            if (!debug_sd_initialised)
            {
                initSD();
            }
            // Serial.println("Saving All Steps (Original / downsize) to SD Card. Unplug the NeoPixel Jumper! 1s");
            // delay(1000);
            debug_no_closest_color = true;
            debug_sd_save_edit = true;
            debug_sd_save_orig = true;
            debug_no_brimap = true;
            countEEPROM();
        }
        else if (s == 'B')
        {
            if (!debug_sd_initialised)
            {
                initSD();
            }
            // Serial.println("Saving All Steps (Original / downsize / Brimap) to SD Card. Unplug the NeoPixel Jumper! 1s");
            // delay(1000);
            debug_no_closest_color = true;
            debug_sd_save_edit = true;
            debug_sd_save_orig = true;
            debug_no_brimap = false;
            countEEPROM();
        }
        else if (s == 'C')
        {
            if (!debug_sd_initialised)
            {
                initSD();
            }
            // Serial.println("Saving All Steps (Original / downsize + brimap + color) to SD Card. Unplug the NeoPixel Jumper! 1s");
            // delay(1000);
            debug_no_closest_color = false;
            debug_sd_save_edit = true;
            debug_sd_save_orig = true;
            debug_no_brimap = false;
            countEEPROM();
        }
        else if (s == 'o')
        {
            if (!debug_sd_initialised)
            {
                initSD();
            }
            // Serial.println("Saving Original Framebuffer to SD Card. Unplug the NeoPixel Jumper! 1s");
            // delay(1000);
            debug_no_closest_color = false;
            debug_sd_save_edit = false;
            debug_sd_save_orig = true;
            debug_no_brimap = false;
            countEEPROM();
        }
        else if (s == 'w')
        {
            if (!debug_sd_initialised)
            {
                initSD();
            }
            // Serial.println("Saving downsized Framebuffer to SD Card. Unplug the NeoPixel Jumper! 1s");
            // delay(1000);
            debug_no_closest_color = true;
            debug_sd_save_edit = true;
            debug_sd_save_orig = false;
            debug_no_brimap = true;
            countEEPROM();
        }
        else if (s == 'b')
        {
            if (!debug_sd_initialised)
            {
                initSD();
            }
            // Serial.println("Saving brimap Framebuffer to SD Card. Unplug the NeoPixel Jumper! 1s");
            // delay(1000);
            debug_no_closest_color = true;
            debug_sd_save_edit = true;
            debug_sd_save_orig = false;
            debug_no_brimap = false;
            countEEPROM();
        }
        else if (s == 'c')
        {
            if (!debug_sd_initialised)
            {
                initSD();
            }
            // Serial.println("Saving Colorized Framebuffer to SD Card. Unplug the NeoPixel Jumper! 1s");
            // delay(1000);
            debug_no_closest_color = false;
            debug_sd_save_edit = true;
            debug_sd_save_orig = false;
            debug_no_brimap = false;
            countEEPROM();
        }
        else if (s == 'e')
        {
            EEPROM.write(0, 0);
            EEPROM.commit();
        }
        else if (s == 'h')
        {
            Serial.println("");
            Serial.println("*~~~~~~~~~~*");
            Serial.println("Help: This shows the different commands (commands are case sensitive)");
            Serial.println("\r\nCommon command Combo: owbc  |  saves all steps\r\n");
            Serial.println("d: toggles Debug mode");
            Serial.println("i: Initialises SD Card");
            Serial.println("r: recalibrates: White image as brimap -> Substracting");
            Serial.println("s: Saves the current Brimap to SD");
            Serial.println("W: Saves Original + Downsize to SD");
            Serial.println("B: Saves Original + Downsize/Brimap to SD");
            Serial.println("C: Saves Original + Downsize/Brimap/Color to SD");
            Serial.println("o: Saves Original to SD");
            Serial.println("w: Saves Downsize to SD");
            Serial.println("b: Saves Downsize/Brimap to SD");
            Serial.println("c: Saves Downsize/Brimap/Color to SD");
            Serial.println("e: Resets EEPROM to 0");
            Serial.println("p: prints whole content of eeprom");
            Serial.println("h: Shows this help");
            Serial.println("*~~~~~~~~~~*");
            Serial.println("");
        }
        else if (s == 's')
        {
            Serial.println("Saving Brimap to sd");
            initSD();
            countEEPROM();
            saveImgToSdPPM(brimap, END_RESOLUTION * END_RESOLUTION * 3, "BRIMAP");
        }
        else if (s == 'p')
        {
            initEEPROM();
            Serial.println("");
            /*
            Serial.println(EEPROM.read(0));
            for (int j = 0; j < H10; j++)
            {
                for (int i = 0; i < W10; i++)
                {
                    Serial.printf("%03d ", EEPROM.read(W10*j + i + 1));
                }
                Serial.println("");
            }
            */
            for (int i = 0; i < EEPROM_SIZE; i++)
            {
                Serial.printf("%03d ", EEPROM.read(i));
            }
            Serial.println("");
        }
        else if (s == 'r')
        {
            calibrate_brimap();
        }
    }
}


void print_frame(uint8_t frame[END_RESOLUTION][END_RESOLUTION][3])
{
    Serial.println("-StartOfFrame-");
    for (int y = 0; y < END_RESOLUTION; y++)
    {
        for (int x = 0; x < END_RESOLUTION; x++)
        {
            Serial.printf("%#02x %#02x %#02x   ", frame[y][x][0], frame[y][x][1], frame[y][x][2]);
        }
        Serial.println();
    }
    Serial.println("-EndOfFrame-");
}

void print_whole_frame(uint8_t *frame)
{
    Serial.println("-StartOfFrame-");
    int z = 0;
    for (int y = 0; y < IMAGE_HEIGHT; y++)
    {
        for (int x = 0; x < IMAGE_WIDTH; x++)
        {
            Serial.printf("%#02x %#02x %#02x   ", frame[z++], frame[z++], frame[z++]);
        }
        Serial.println();
    }
    Serial.println("-EndOfFrame-");
}

#endif
