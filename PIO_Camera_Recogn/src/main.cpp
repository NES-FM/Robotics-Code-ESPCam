#include <Arduino.h>

#include "Adafruit_NeoPixel.h"
#include <math.h>
#include "soc/soc.h"          // Disable brownout problems
#include "soc/rtc_cntl_reg.h" // Disable brownout problems
#include "driver/rtc_io.h"

bool debug_no_closest_color = false;
bool debug = false;
bool debug_enabled = false;

bool debug_no_brimap = false;

bool debug_sd_save_orig = false;
bool debug_sd_save_edit = false;

bool debug_linerecogn_return = false;

bool calibrateBrimap = false;

int last_debug_millis = 0;

#include "config.h"
#include "cuart.h"
#include "save.h"
#include "image_helpers.h"
#include "serial_helpers.h"
#include "recogn.h"

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(4, 16, NEO_GRB + NEO_KHZ800);

void setup() 
{
    pinMode(33, OUTPUT);
    digitalWrite(33, LOW);
    delay(500);
    digitalWrite(33, HIGH);

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    pinMode(12, INPUT_PULLUP);

    pixels.begin();
    pixels.show();

    pixels.fill(pixels.Color(156, 156, 156));
    pixels.show();

    init_camera();

    cuart_init();

    loadBrimap();

    // Recalibrate button
    pinMode(2, INPUT_PULLUP);

    Serial.print("Main started on core ");
    Serial.println(xPortGetCoreID());
}

unsigned long mil_s = 0;
unsigned long mil_1 = 0;
unsigned long mil_2 = 0;
unsigned long mil_3 = 0;
unsigned long mil_4 = 0;
unsigned long mil_5 = 0;
unsigned long mil_6 = 0;

void loop() 
{
    serial_interface_tick();
    if (debug_enabled && (millis() - last_debug_millis) >= 500)
    {
        last_debug_millis = millis();
        debug = true;
    }
    else
    {
        debug = false;
    }

    mil_s = millis();

    camera_fb_t *fb = esp_camera_fb_get();

    if (debug_sd_save_orig)
    {
        saveImgToSd(fb->buf, fb->len, "1_Original");
        debug_sd_save_orig = false;
    }

    fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, temp_buffer);

    esp_camera_fb_return(fb);

    mil_1 = millis();

    downsize(temp_buffer, IMAGE_WIDTH * IMAGE_HEIGHT * 3);

    mil_2 = millis();

    if (debug_sd_save_edit)
    {
        String r = debug_no_closest_color ? "raw_" : "color_";
        r += debug_no_brimap ? "nobri" : "bri";
        saveImgToSdPPM(rgb_frame, END_RESOLUTION * END_RESOLUTION * 3, "2_Edit_" + r);
        debug_sd_save_edit = false;
    }

    mil_3 = millis();

    debug_linerecogn_return = line_recogn(rgb_frame);

    mil_4 = millis();

    if (debug)
    {
        Serial.print("Timing: Total: ");
        Serial.print(mil_4 - mil_s);
        Serial.print(" Recognition: ");
        Serial.print(mil_4 - mil_3);
        Serial.print(" Downsize: ");
        Serial.print(mil_2 - mil_1);
        Serial.print(" Taking Image: ");
        Serial.println(mil_1 - mil_s);
        Serial.println();
    }

    if (!debug_linerecogn_return && debug)
        Serial.println("!! Line Recogn. returned False !!");

    if (digitalRead(2) == LOW) 
    {
        calibrate_brimap();
        while(digitalRead(2) == LOW) {}
    }


    debug_no_closest_color = false;
    debug_no_brimap = false;
}