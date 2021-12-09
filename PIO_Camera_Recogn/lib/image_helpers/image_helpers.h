#ifndef IMAGE_HELPERS_H
#define IMAGE_HELPERS_H

#include <Arduino.h>
#include "esp_camera.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

#include "config.h"

#include "save.h"

#define b_w 65
#define g_b_r -1
#define g_b_g 20
#define g_b_b -1

uint8_t rgb_frame[END_RESOLUTION][END_RESOLUTION][3] = {0};
//uint8_t rgb_frame[HEIGHT][WIDTH][3] = {0};

uint8_t *temp_buffer = (uint8_t *)calloc(IMAGE_WIDTH * IMAGE_HEIGHT * 3, sizeof(uint8_t));
size_t temp_buffer_len = IMAGE_WIDTH * IMAGE_HEIGHT * 3;

uint8_t *small_buffer = (uint8_t *)calloc(END_RESOLUTION * END_RESOLUTION * 3, sizeof(uint8_t));
size_t small_buffer_len = END_RESOLUTION * END_RESOLUTION * 3;

uint8_t *small_jpg_buffer = (uint8_t *)calloc(END_RESOLUTION * END_RESOLUTION * 3, sizeof(uint8_t));
size_t small_jpg_buffer_len = 0;


const int distinctRGB[6][3] = {{5, 5, 5}, {200, 200, 200}, {90, 130, 90}};
const String distinctColors[6] = {"black"
                                  "white",
                                  "green"};

void init_camera();
String closestColor(int r, int g, int b);
void calibrate_brimap();
void downsize(uint8_t *buf, size_t len);

void init_camera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = IMAGE_RESOLUTION;
    config.jpeg_quality = 10; // 10-63, lower number means higher quality
    config.fb_count = 1;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();

    //drop down frame size for higher initial frame rate
    //   s->set_framesize(s, IMAGE_RESOLUTION);
    s->set_contrast(s, 2);
    s->set_lenc(s, false);
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);    
}

String closestColor(int r, int g, int b)
{
    if (r > 200 && g > 200 && b > 200)
    {
        if (((r + b) / 2) < (g - 10))
            return "green";
        else
            return "white";
    }
    else
    {
        if (((r + b) / 2) < (g - 20))
            return "green";
        else
            return "black";
    }
}

void calibrate_brimap()
{
    initEEPROM();
    //calibrateBrimap = true;
    debug_no_closest_color = true;
    debug_sd_save_edit = false;
    debug_sd_save_orig = false;
    debug_no_brimap = true;

    Serial.println("Starting Calibration");

    camera_fb_t *fb = esp_camera_fb_get();
    fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, temp_buffer);
    esp_camera_fb_return(fb);
    downsize(temp_buffer, IMAGE_WIDTH * IMAGE_HEIGHT * 3);
    Serial.println("Starting memcpy");
    // memcpy(brimap, rgb_frame, END_RESOLUTION*END_RESOLUTION*3);
    for (int col = 0; col < END_RESOLUTION; col++)
    {
        for (int row = 0; row < END_RESOLUTION; row++)
        {
            brimap[col][row][0] = constrain(rgb_frame[col][row][0] + 10, 0, 255);
            brimap[col][row][1] = constrain(rgb_frame[col][row][1] + 10, 0, 255);
            brimap[col][row][2] = constrain(rgb_frame[col][row][2] + 10, 0, 255);
        }
    }
    saveBrimap();
    Serial.println("Done Saving the BRIMAP");
}

///*
// Downsize using blocks, which are averaged individually. + 1:1
void downsize(uint8_t *buf, size_t len)
{
    // Looping through each block: Getting the top Left coordinate
    for (uint8_t block_in_image_y = 0; block_in_image_y < IMAGE_HEIGHT; block_in_image_y += PER_BLOCK)
    {
        for (uint8_t block_in_image_x = OFFSET_RIGHT; block_in_image_x < (IMAGE_HEIGHT + OFFSET_RIGHT); block_in_image_x += PER_BLOCK)
        {
            int block_r = 0;
            int block_g = 0;
            int block_b = 0;

            // Loobing through each pixel in the block: The Offset from the top left corner
            for (uint8_t offset_in_block_y = 0; offset_in_block_y < PER_BLOCK; offset_in_block_y++)
            {
                for (uint8_t offset_in_block_x = 0; offset_in_block_x < PER_BLOCK; offset_in_block_x++)
                {
                    // Calculating the Index into the buffer
                    size_t index = 3 * ((block_in_image_x + offset_in_block_x) + ((block_in_image_y + offset_in_block_y) * IMAGE_WIDTH));
                    block_b += buf[index + 0];
                    block_g += buf[index + 1];
                    block_r += buf[index + 2];
                }
            }

            // Calculating the Average: There are PER_BLOCK ^2 Pixels in each block, so divide it by that and take only the int
            block_r = int(block_r / (PER_BLOCK * PER_BLOCK));
            block_g = int(block_g / (PER_BLOCK * PER_BLOCK));
            block_b = int(block_b / (PER_BLOCK * PER_BLOCK));

            // Calculating the Block coordinate (for example the second block from the top left would have the coordinates 1 , 0)
            size_t block_x = int((block_in_image_x - OFFSET_RIGHT) / PER_BLOCK);
            size_t block_y = int(block_in_image_y / PER_BLOCK);

            // Variables for storing the color
            uint8_t rn;
            uint8_t gn;
            uint8_t bn;

            if (!debug_no_brimap)
            {
                // If the Brightmap is enabled (default)
                rn = map(constrain(brimap[block_y][block_x][0] - block_r, 0, 255), 0, 255, 255, 0); //brimap has calibration data, r contains value, map inverts the color
                gn = map(constrain(brimap[block_y][block_x][1] - block_g, 0, 255), 0, 255, 255, 0); //brimap has calibration data, g contains value, map inverts the color
                bn = map(constrain(brimap[block_y][block_x][2] - block_b, 0, 255), 0, 255, 255, 0); //brimap has calibration data, b contains value, map inverts the color
            }
            else
            {
                // If the Brightmap is disabled
                rn = block_r;
                gn = block_g;
                bn = block_b;
            }

            if (debug_no_closest_color)
            {
                // If the closest color routine is disabled
                rgb_frame[block_y][block_x][0] = rn;
                rgb_frame[block_y][block_x][1] = gn;
                rgb_frame[block_y][block_x][2] = bn;
            }
            else
            {
                // If the closest color routine is enabled (default)
                String colorReturn = closestColor(rn, gn, bn);
                uint8_t color[3];
                if (colorReturn == "black")
                {
                    color[0] = 0;
                    color[1] = 0;
                    color[2] = 0;
                }
                else if (colorReturn == "white")
                {
                    color[0] = 255;
                    color[1] = 255;
                    color[2] = 255;
                }
                else if (colorReturn == "green")
                {
                    color[0] = 0;
                    color[1] = 255;
                    color[2] = 0;
                }
                else
                {
                    color[0] = 0;
                    color[1] = 0;
                    color[2] = 0;
                }
                rgb_frame[block_y][block_x][0] = color[0];
                rgb_frame[block_y][block_x][1] = color[1];
                rgb_frame[block_y][block_x][2] = color[2];
                // Serial.printf("Y: %2d, X: %2d, I: %6d, R: %3d, G: %3d, B:
                // %3d, C: %s\n", block_y, block_x, i, rn, gn, bn,
                // colorReturn);
            }
        }
    }
}
//*/

#endif
