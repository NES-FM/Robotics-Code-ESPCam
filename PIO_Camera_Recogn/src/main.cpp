#include <Arduino.h>

#include "pin_definitions.h"

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

bool debug_serial_print_modified_frame = false;

bool debug_linerecogn_return = false;

bool calibrateBrimap = false;

int last_debug_millis = 0;

#include "config.h"
#include "cuart.h"
#include "save.h"
#include "image_helpers.h"
#include "serial_helpers.h"
#include "recogn.h"

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(4, PIN_NEO_SHIFT, NEO_GRB + NEO_KHZ800);

void setup() 
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    pinMode(PIN_DIP_WETTKAMPFMODUS, INPUT_PULLUP);

    // Temporärer test für weiße LED

    pixels.begin();
    pixels.show();

    pixels.fill(pixels.Color(156, 156, 156));
    pixels.show();
    // pinMode(4, OUTPUT);
    // digitalWrite(4, HIGH);

    init_camera();

    cuart_init();

    loadBrimap();

    // Recalibrate button
    pinMode(PIN_CAMCALIB, INPUT_PULLUP);

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
    if (debug_enabled && (millis() - last_debug_millis) >= 1000)
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

    if (digitalRead(PIN_CAMCALIB) == LOW) 
    {
        calibrate_brimap();
        while(digitalRead(PIN_CAMCALIB) == LOW) {}
    }

    if (debug_serial_print_modified_frame)
    {
        print_frame(rgb_frame);
        debug_serial_print_modified_frame = false;
    }


    debug_no_closest_color = false;
    debug_no_brimap = false;
}