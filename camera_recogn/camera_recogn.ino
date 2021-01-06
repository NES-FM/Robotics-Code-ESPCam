#include "Adafruit_NeoPixel.h"
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(4, 13, NEO_GRB + NEO_KHZ800);

#include <Wire.h>

#include <math.h>

#include "esp_camera.h"
// Select camera model (Uncomment in camera_pins.h first)
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

/*
// const char* ssid = "ESP32-AAAAA";
const char* ssid = "AMHOME";
const char* password = "DasIstMeinPreSharedKey07";
*/

bool raw = false;

#define WIDTH 160
#define HEIGHT 120
#define BLOCK_SIZE 10
//#define W (WIDTH / BLOCK_SIZE)
#define W ((WIDTH / BLOCK_SIZE)-4)
#define H (HEIGHT / BLOCK_SIZE)

#define W10 (WIDTH / 10)
#define H10 (HEIGHT / 10)

String line = "";

uint8_t rgb_frame[H][W][3] = { 0 };
//uint8_t rgb_frame[HEIGHT][WIDTH][3] = {0};

uint8_t * temp_buffer = (uint8_t *)calloc(WIDTH * HEIGHT * 3,sizeof(uint8_t));
size_t temp_buffer_len = 0;


void i2c_write_register(int DEVICE_I2C_ADDRESS, int DEVICE_REGISTER, String toWrite)
{
    Wire.beginTransmission(DEVICE_I2C_ADDRESS); // select device with "beginTransmission()"
    Wire.write(DEVICE_REGISTER); // select starting register with "write()"
    Wire.write(toWrite);
    Wire.endTransmission(); // end write operation, as we just wanted to select the starting register
}


void setup() {  

    Wire.begin(14, 15, 8000000);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  pinMode(12, INPUT_PULLUP);

  pixels.begin();
  pixels.show();

  pixels.fill(pixels.Color(255, 255, 255));
  pixels.show();

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
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QQVGA);
  s->set_contrast(s, 2);
  s->set_lenc(s, false);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

/*
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
*/
}


const int distinctRGB[6][3]    = {{5, 5, 5}, {200, 200, 200}, {90, 130, 90}};
const String distinctColors[6] = {"black"   "white"        , "green"     };
/*
String closestColor(int r,int g,int b) {
  String colorReturn = "NA";
  int biggestDifference = 1000;
  for (int i = 0; i < 6; i++) {
    if (sqrt(pow(r - distinctRGB[i][0],2) + pow(g - distinctRGB[i][1],2) + pow(b - distinctRGB[i][2],2)) < biggestDifference) {
      colorReturn = distinctColors[i];
      biggestDifference = sqrt(pow(r - distinctRGB[i][0],2) + pow(g - distinctRGB[i][1],2) + pow(b - distinctRGB[i][2],2));
    }
  }
  return colorReturn;
}
//*/

#define b_w 65
#define g_b_r -1
#define g_b_g 20
#define g_b_b -1
///*
String closestColor(int r,int g,int b) {
  if((r < b_w && b < b_w)) // -----------g < b_w && --------------------------------- black or green
  {
      if(r == 0 && g == 0 && b == 0) // --------------------------------------------- black
      {
          return "black";
      }
      else if((r > g_b_r && g > g_b_g && b > g_b_b) && (g >= b && r <= (0.8*g))) // - green
      {
          return "green";
      }
      else // ----------------------------------------------------------------------- other black
      {
          return "black";
      }
  }
  else // --------------------------------------------------------------------------- white
  {
      return "white";
  }
  return "NA";
}
//*/
/*
//Manually Generated
int brimap[12][12] = {
    {30, 25 , 15 , 10 , 0  , 0  , 0  , 0  , 15 , 20 , 30 , 35}, //done
    {25, 15 , 5  , 0  , 0  , 0  , 0  , 0  , 0  , 10 , 20 , 30}, //done
    {15, 5  , 0  , 0  , 133, 138, 138, 133, 0  , 0  , 10 , 20}, //done
    {10, 0  , 133, 133, 150, 155, 155, 150, 133, 133, 0  , 15},
    {5 , 133, 150, 160, 170, 180, 180, 170, 160, 150, 133, 10},
    {0 , 155, 162, 170, 180, 200, 200, 185, 170, 158, 0  , 5}, //done //-mid
    {0 , 155, 162, 170, 180, 200, 200, 185, 170, 158, 0  , 5}, //done \\-mid
    {5 , 133, 150, 160, 170, 180, 180, 170, 160, 150, 133, 10},
    {10, 0  , 133, 133, 150, 155, 155, 150, 133, 133, 0  , 15},
    {15, 5  , 0  , 0  , 133, 138, 138, 133, 0  , 0  , 10 , 20}, //done
    {25, 15 , 5  , 0  , 0  , 0  , 0  , 0  , 0  , 10 , 20 , 30}, //done
    {30, 25 , 15 , 10 , 0  , 0  , 0  , 0  , 10 , 20 , 30 , 35} //done
};*/
/*
//Auto Generated with Python
int brimap[12][12] = {
  {161, 62, 37, 12, -2, 3, 0, -3, 7, 28, 52, 122},
  {137, 24, 16, 2, -7, -3, -1, -4, -47, -30, 10, 80},
  {81, 15, -4, -27, -40, -14, -16, -23, -51, -49, -37, 43},
  {66, 3, -24, -52, -68, -65, -59, -52, -53, -54, -50, 0},
  {70, -2, -33, -67, -91, -108, -126, -101, -54, -54, -50, -13},
  {0, 0, -16, -59, -104, -163, -182, -140, -92, -43, -1, 3},
  {1, 1, -12, -63, -113, -176, -189, -153, -91, -47, 1, 3},
  {-1, -8, -24, -58, -97, -132, -151, -115, -72, -41, -10, 5},
  {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
  {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
  {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
  {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}
};
*/
///*
/*
//2nd Auto generated
int brimap[12][12] = {
  {200, 168, 153, 131, 117, 105, 100, 104, 113, 136, 155, 193},
  {186, 142, 111, 90, 75, 64, 57, 61, 68, 83, 111, 175},
  {174, 116, 79, 53, 33, 20, 15, 14, 25, 44, 76, 138},
  {151, 97, 57, 25, -2, -26, -37, -32, -15, 14, 48, 103},
  {131, 83, 41, 0, -38, -51, -51, -52, -48, -12, 32, 79},
  {128, 75, 33, -14, -52, -51, -52, -51, -52, -37, 21, 65},
  {132, 74, 33, -17, -52, -51, -54, -51, -52, -39, 19, 62},
  {152, 82, 38, -5, -46, -52, -52, -52, -53, -22, 28, 72},
  {167, 93, 51, 16, -11, -37, -50, -46, -28, 3, 43, 89},
  {177, 109, 70, 42, 20, 3, -5, -3, 11, 31, 63, 119},
  {192, 131, 96, 66, 50, 41, 37, 38, 48, 65, 94, 155},
  {190, 192, 151, 157, 148, 149, 153, 167, 179, 144, 116, 157},
};
*/
/*
//AutoGenerated 3
int brimap[12][12] = {
  {199, 166, 148, 124, 108, 99, 94, 95, 105, 123, 145, 186},
  {189, 142, 107, 86, 70, 58, 53, 57, 64, 77, 101, 149},
  {179, 118, 77, 49, 28, 19, 11, 11, 21, 39, 70, 112},
  {171, 102, 57, 24, -2, -26, -39, -36, -15, 15, 45, 92},
  {164, 87, 44, 3, -36, -52, -52, -52, -52, -14, 36, 76},
  {162, 81, 39, -13, -52, -52, -54, -52, -53, -36, 23, 68},
  {163, 81, 39, -10, -52, -52, -54, -52, -53, -36, 21, 64},
  {168, 89, 47, 5, -37, -53, -52, -53, -52, -17, 32, 75},
  {177, 104, 62, 29, -1, -26, -40, -36, -17, 9, 48, 93},
  {200, 129, 88, 56, 32, 19, 14, 16, 24, 42, 74, 121},
  {200, 150, 119, 90, 74, 64, 58, 60, 69, 84, 112, 157},
  {199, 181, 153, 133, 119, 108, 101, 106, 115, 132, 159, 190},
};
*/
/*
//autogenerated 4
int brimap[12][12] = {
  {123, 51, 23, 0, -20, 4, 4, 4, -21, -3, 26, 73},
  {91, 18, -20, -47, -52, 3, 3, 2, -53, -47, -21, 20},
  {75, 6, -47, -50, -54, -14, -3, 0, -54, -38, -2, 34},
  {63, -22, -50, -54, -53, -73, -76, -61, -90, -62, -27, 20},
  {54, -31, -51, -54, -54, -141, -159, -125, -114, -85, -41, 8},
  {54, -47, -51, -53, -54, -183, -197, -174, -133, -101, -48, -1},
  {2, -13, -41, -80, -113, -160, -194, -164, -98, -39, 0, 2},
  {3, 2, 4, -1, -30, -94, -126, -102, -49, -8, 3, 3},
  {5, 3, 2, 2, 0, -27, -40, -29, -9, 1, 0, 5},
  {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
  {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
  {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
};
*/
/*
// AutoGenerated + Manual Symetry
int brimap[12][12] = {
  {172, 101, 74, 50, 0, 0, 0, 28, 26, 45, 74, 124},
  {141, 70, 46, 10, -2, 0, -4, -47, 23, 32, 54, 71},
  {140, 47, 3, 3, -25, -24, -33, -41, -19, -3, 30, 72},
  {113, 24, 4, 2, -56, -77, -92, -75, -49, -27, 7, 114},
  {104, 12, 2, 1, -119, -141, -162, -114, -80, -49, -5, 9},
  {106, 6, 2, 2, -136, -188, -208, -133, -94, -55, 12, 68},
  {-2, -18, -30, -52, -133, -171, -210, -167, -111, -55, -10, 0},
  {0, 0, -4, -22, -157, -107, -134, -109, -61, -31, -4, -1},
  {-3, -5, -15, -22, -133, -43, -53, -52, -41, -24, -11, 0},
  {140, 47, 3, 3, -25, -24, -33, -41, -19, -3, 30, 72},
  {141, 70, 46, 10, -2, 0, -4, -47, 23, 32, 54, 71},
  {172, 101, 74, 50, 0, 0, 0, 28, 26, 45, 74, 124}
};
//*/

/*
//AutoGenerated 5
int brimap[12][12] = {
  {228, 151, 113, 82, 58, 0, 0, -5, 35, 59, 74, 122},
  {197, 114, 75, 35, 11, -1, 0, -12, 4, 4, 34, 83},
  {171, 84, 37, 4, 2, -13, -11, -32, 2, 3, 4, 45},
  {153, 59, 6, 3, 0, -30, -32, -64, 1, 4, 3, 2},
  {119, -6, -18, -48, -75, -102, -137, -143, -109, -74, -37, -10},
  {97, 0, -3, -27, -90, -151, -195, -181, -129, -78, -24, -1},
  {95, 0, -7, -32, -94, -164, -215, -203, -143, -75, -24, 0},
  {142, -2, -13, 52, 9, -126, -181, -136, -101, -65, -30, 7},
  {144, 50, 3, 1, 2, -83, -99, -93, -59, -37, -6, 32},
  {159, 74, 25, 3, 0, -43, -34, -36, -26, -7, 21, 43},
  {184, 105, 66, 19, 2, -14, -9, -11, 5, 20, 49, 77},
  {202, 149, 96, 64, 40, -10, -2, -1, 16, 31, 77, 128},
};
//*/

/*
// AutoGenerated 6 <-
int brimap[12][12] = {
  {163, 109, 83, 66, 47, 0, 0, -8, 57, 78, 109, 215},
  {142, 72, 35, 6, 2, -2, 0, -6, 5, 34, 70, 141},
  {108, 37, 4, 3, 1, -25, -18, -19, 3, 4, 35, 125},
  {87, 6, 3, 1, 0, -62, -55, -47, 2, 2, 11, 112},
  {67, 3, 0, 2, 5, -134, -125, -86, 10, 20, 13, 92},
  {-1, -13, -42, -95, -152, -196, -175, -132, -59, -18, -2, 0},
  {0, -4, -31, -90, -157, -202, -189, -144, -52, -19, 0, 0},
  {-4, 0, -21, -59, -111, -159, -143, -121, -56, -27, 0, 0},
  {101, 44, 3, -23, -47, -72, -70, -63, 2, 2, 16, 77},
  {122, 59, 29, 1, -21, -30, -28, -30, 2, 3, 31, 113},
  {137, 75, 57, 32, 13, -7, -11, -12, 2, 11, 55, 145},
  {191, 90, 95, 85, 63, -1, -1, -1, 63, 85, 125, 201},
};
//*/

/*
//AutoGenerated 7
int brimap[12][12] = {
  {-88, -156, -237, -17, -7, -11, -10, -8, -7, -3, -2, -2},
  {-223, 0, -5, -17, -22, -31, -39, -45, -35, -26, -16, -12},
  {-254, -1, -8, -33, -52, -58, -48, -59, -63, -53, -38, -24},
  {-239, -11, -20, -61, -73, -94, -115, -119, -102, -90, -58, -28},
  {-244, -1, -35, -89, -115, -146, -207, -186, -144, -119, -86, -42},
  {-245, -19, -38, -106, -164, -201, -252, -248, -172, -140, -107, -52},
  {-250, -23, -46, -175, -159, -255, -254, -253, -240, -171, -125, -64},
  {-255, -26, -46, -153, -157, -253, -254, -253, -224, -168, -108, -68},
  {-247, -19, -35, -97, -133, -190, -253, -232, -167, -140, -88, -55},
  {-251, -165, -27, -73, -103, -138, -166, -156, -130, -110, -87, -42},
  {-253, -57, -19, -53, -77, -95, -105, -103, -87, -75, -48, -31},
  {-254, -8, -16, -35, -44, -55, -58, -55, -43, -36, -23, -11},
};
//*/

/*
//AVG
int brimap[12][12] = {
    {83, 49, 3, 109, 110, 105, 103, 104, 105, 110, 116, 120},
    {13, 119, 100, 81, 70, 59, 51, 49, 57, 66, 81, 98},
    {-10, 106, 82, 53, 34, 22, 22, 17, 18, 32, 52, 78},
    {-5, 90, 62, 22, 3, -18, -34, -36, -23, -6, 27, 61},
    {-19, 82, 42, -6, -35, -63, -99, -89, -62, -36, -2, 45},
    {-25, 67, 32, -25, -71, -99, -124, -123, -84, -57, -20, 33},
    {-32, 64, 24, -66, -75, -126, -126, -125, -119, -79, -33, 23},
    {-33, 64, 24, -53, -74, -125, -126, -125, -111, -77, -23, 22},
    {-22, 75, 38, -19, -53, -94, -125, -115, -82, -54, -8, 34},
    {-17, 13, 49, 3, -25, -53, -74, -67, -48, -28, 2, 51},
    {-11, 80, 66, 33, 7, -12, -22, -21, -8, 7, 38, 68},
    {-4, 114, 84, 62, 48, 34, 29, 31, 40, 48, 66, 91}
};
//*/

/*
//AutoGenerated 7 refined backup (current, relatively good, problem recogn. lücken)
int brimap[12][12] = {
  //{-2, -2, -3, -17, -7, -11, -10, -8, -7, -3, -2, -2},
    //  0     1     2     3 |   4     5     6     7     8 |   9    10    11
    {  80,   80,   79,   64,   35,   14,    5,    7,   15,   35,   80,   80}, //  0
    { -12,    0,   -5,  -17,  -22,  -31,  -39,  -45,  -35,  -26,  -16,  -12}, //  1
    { -23,   -1,   -8,  -33,  -52,  -58,  -48,  -59,  -63,  -53,  -38,  -24}, //  2
    { -28,  -11,  -20,  -61,  -73,  -94, -115, -119, -102,  -90,  -58,  -28}, //  3
    { -42,   -1,  -35,  -89, -115, -146, -207, -186, -144, -119,  -86,  -42}, //  4
    { -52,  -19,  -38, -106, -164, -201, -252, -248, -172, -140, -107,  -52}, //  5
    { -65,  -23,  -46, -175, -159, -255, -254, -253, -240, -171, -125,  -64}, //  6
    { -68,  -26,  -46, -153, -157, -253, -254, -253, -224, -168, -108,  -68}, //  7
    { -55,  -19,  -35,  -97, -133, -190, -253, -232, -167, -140,  -88,  -55}, //  8
    { -42,  -87,  -27,  -73, -103, -138, -166, -156, -130, -110,  -87,  -42}, //  9
    { -31,  -48,  -19,  -53,  -77,  -95, -105, -103,  -87,  -75,  -48,  -31}, // 10
    { -11,   -8,  -16,  -35,  -44,  -55,  -58,  -55,  -43,  -36,  -23,  -11}, // 11
};
//*/

/*
//AutoGenerated 7 refined some more (current, relatively good, problem recogn. lücken)
int brimap[12][12] = {
  //{-2, -2, -3, -17, -7, -11, -10, -8, -7, -3, -2, -2},
    //  0     1     2     3 |   4     5     6     7     8 |   9    10    11
    {  80,   80,   79,   64,   35,   14,    5,    7,   15,   35,   80,   80}, //  0
    { -12,    0,    0,  -17,  -22,  -31,  -39,  -45,  -35,  -26,   -6,   23}, //  1
    { -23,   -1,   -8,  -33,  -52,  -58,  -48,  -59,  -63,  -53,  -38,   -5}, //  2
    { -28,  -11,  -20,  -61,  -73,  -94, -115, -119, -102,  -90,  -58,  -28}, //  3
    { -42,   -1,  -35,  -89, -115, -146, -207, -186, -144, -119,  -86,  -42}, //  4
    { -52,  -19,  -38, -106, -164, -201, -252, -248, -172, -140, -107,  -52}, //  5
    { -65,  -23,  -46, -175, -159, -255, -254, -253, -240, -171, -125,  -64}, //  6
    { -68,  -26,  -46, -153, -157, -253, -254, -253, -224, -168, -108,  -68}, //  7
    { -55,  -19,  -35,  -97, -133, -190, -253, -232, -167, -140,  -88,  -55}, //  8
    { -42,  -87,  -27,  -73, -103, -138, -166, -156, -130, -110,  -87,  -42}, //  9
    { -31,  -48,  -19,  -53,  -77,  -95, -105, -103,  -87,  -75,  -48,  -21}, // 10
    { -11,   -8,  -16,  -35,  -44,  -55,  -58,  -55,  -43,  -36,  -23,    5}, // 11
};
//*/

///*
// AutoGen 8 black <<< works the best
int brimap[12][12] = {
  // 0    1     2     3 |   4     5     6     7     8 |   9   10   11
  {255,  60,   50,   40,   30,   20,   20,   30,   40,   50,  60, 255}, //  0
  {255,  20,  -13,  -21,  -22,  -27,  -26,  -26,  -20,  -11,  20, 255}, //  1
  {255, -15,  -40,  -49,  -58,  -73,  -58,  -59,  -48,  -31,  -8, 255}, //  2
  {255, -28,  -69,  -84,  -86, -123, -105,  -95,  -95,  -64, -24, 255}, //  3
  {255, -49,  -99, -116, -150, -188, -174, -155, -120,  -99, -43, 255}, //  4
  {255, -65, -131, -171, -195, -194, -193, -185, -161, -110, -54, 255}, //  5
  {255, -86, -135, -198, -195, -194, -192, -198, -179, -117, -65, 255}, //  6
  {255, -88, -131, -174, -195, -194, -192, -188, -165,  -91, -66, 255}, //  7
  {255, -62, -111, -158, -175, -194, -194, -169, -147,  -85, -48, 255}, //  8
  {255, -43,  -94, -125, -139, -161, -147, -133, -120,  -81, -37, 255}, //  9
  {255,   0,  -57,  -76,  -90, -105,  -96,  -85,  -78,  -48,   0, 255}, // 10
  {255,  46,   37,    0,  -30,  -40,  -30,    0,   20,   33,  53, 255}, // 11
};
//*/

/*
// AutoGen 8 + whte / black combined
int brimap[12][12] = {
  {254, 245, 238, 225, 217, 215, 215, 218, 225, 235, 242, 253},
  {255,-4, -13, -21, -22, -27, -26, -26, -20, -11, 0, 248},
  {252,-15, -40, -49, -58, -73, -58, -59, -48, -31, -8, 234},
  {245,-28, -69, -84, -96, -118, -115, -105, -95, -64, -24, 220},
  {238,-49, -99, -126, -160, -197, -194, -155, -130, -99, -43, 186},
  {235,-65, -131, -168, -232, -254, -253, -172, -161, -130, -54, 175},
  {235,-86, -155, -226, -254, -254, -252, -252, -216, -137, -65, 164},
  {234,-88, -151, -209, -255, -254, -252, -250, -185, -101, -66, 167},
  {236,-62, -121, -158, -185, -254, -205, -169, -147, -102, -48, 197},
  {243,-43, -94, -125, -139, -181, -147, -133, -120, -81, -37, 213},
  {249,-24, -57, -76, -90, -115, -96, -85, -78, -48, -19, 231},
  {254, 190, 161, 140, 124, 115, 119, 123, 135, 158, 194, 244},
};
//*/

//int brimap[12][12] = { 0 };

#define mvr 3

///*
//downsize throwing away pixels + 1:1
void downsize(uint8_t *buf, size_t len) {
    //Serial.print("I: ");
    size_t small_pix = 0;
    for (size_t i = 0; i < len; i += (3 * BLOCK_SIZE)) {
        //Serial.printf("%d ", i);
        const uint8_t r = buf[i+0];
        const uint8_t g = buf[i+1];
        const uint8_t b = buf[i+2];

        const size_t j = floor(i / 3);            //floor unnessecary, but converts float to int for ease
        const size_t pix = floor(j / BLOCK_SIZE); //floor unnessecary, but converts float to int for ease
        const size_t y = floor(j / WIDTH);        //floor NESSECARY, WIDTH is also right here

        if (y % BLOCK_SIZE == 0)
        {
            if (mvr <= (pix % W10) && (pix % W10) <= W10 - (5 - mvr))  // mvr = 2 : 2 <= pix <= 13 (<- 16 - 2)
            {
                small_pix++;
                const uint8_t block_x = small_pix % H10;
                const uint8_t block_y = floor(y / BLOCK_SIZE); //floor unnessecary, but converts float to int for ease
                
                //    // average pixels in block (accumulate)
                //    rgb_frame[block_y][block_x][0] = r;
                //    rgb_frame[block_y][block_x][1] = g;
                //    rgb_frame[block_y][block_x][2] = b;
                //    //Serial.printf("raw - by: %d bx: %d - r: %d g: %d b: %d\n", block_y, block_x, r, g, b);

                uint8_t rn;
                uint8_t gn;
                uint8_t bn;        

                rn = constrain(r + brimap[block_y][block_x], 0, 255);
                gn = constrain(g + brimap[block_y][block_x], 0, 255);
                bn = constrain(b + brimap[block_y][block_x], 0, 255);
                
                if (raw)
                {
                    rgb_frame[block_y][block_x][0] = rn;
                    rgb_frame[block_y][block_x][1] = gn;
                    rgb_frame[block_y][block_x][2] = bn;
                }
                else 
                {
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
                    //Serial.printf("Y: %2d, X: %2d, I: %6d, R: %3d, G: %3d, B: %3d, C: %s\n", block_y, block_x, i, rn, gn, bn, colorReturn);
                }
            }
        }
    }
    //Serial.println();
}

void rad_to_deg(int rad)
{
    return ((rad * 4068) / 71)
}

String line_recogn(uint8_t frame[H][W][3]) {
    int d_x = 0;
    int d_y = 0;
    int angle = 0;

    int start_point[2] = {0, 0};
    bool start_found = false;

    int end_point[2] = {0, 0};
    bool end_found = false;

    int y = 1;

    do
    {
        for (int x = 1; x < W; x++)
        {
            if (frame[y][x][0] == 0 && frame[y][x][1] == 0)
            {
                start_point[0] = y;
                start_point[1] = x;

                start_found = true;
            }

            if (start_found)
            {
                break;
            }
        }

        if (!start_found)
        {
            if (y <= 2)
                y++;
            else
                break;
        }

    } while(!start_found);

    y = 10;

    do
    {
        for (int x = 1; x < W; x++)
        {
            if (frame[y][x][0] == 0 && frame[y][x][1] == 0)
            {
                end_point[0] = y;
                end_point[1] = x;

                end_found = true;
            }

            if (end_found)
            {
                break;
            }
        }

        if (!end_found)
        {
            if (y >= 1)
                y--;
            else 
                break;
        }

    } while(!end_found);

    if (start_found && end_found)
    {
        d_x = end_point[1] - start_point[1];
        d_y = end_point[0] - start_point[0];

        angle = constrain(round(rad_to_deg(atan2(d_y, d_x)) - 90), -90, 90);

        if (angle != -90 && angle != 90)
            //sprintf(line, "|%02d", angle);
            line = "||" + String(angle);
        else
            //sprintf(line, "---");
            line = "----";

        Serial.printf("--Debug--  S_point: %02d | %02d  E_point: %02d | %02d  d: %02d | %02d  angle: %03d \n\r", start_point[0], start_point[1], end_point[0], end_point[1], d_y, d_x, angle);
    }
    else
    {

        line = "0000";
    }

    return line;
}

void print_frame(uint8_t frame[H][W][3]) {
    Serial.println("-StartOfFrame-");
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            Serial.printf("%#02x %#02x %#02x   ", frame[y][x][0], frame[y][x][1], frame[y][x][2]);
        }
        Serial.println();
    }
    Serial.println("-EndOfFrame-");
}

void print_whole_frame(uint8_t * frame) {
    Serial.println("-StartOfFrame-");
    int z = 0;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            Serial.printf("%#02x %#02x %#02x   ", frame[z++], frame[z++], frame[z++]);
        }
        Serial.println();
    }
    Serial.println("-EndOfFrame-");
}

unsigned long mil_s = 0;
unsigned long mil_1 = 0;
unsigned long mil_2 = 0;
unsigned long mil_3 = 0;
unsigned long mil_4 = 0;
unsigned long mil_5 = 0;
unsigned long mil_6 = 0;

void loop() {
    /*
    if(digitalRead(12) == LOW)
    {
        camera_fb_t  * fb = esp_camera_fb_get();
        delay(15);
        bool success = false;
        success = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, temp_buffer);
        esp_camera_fb_return(fb);
        if (!success)
        {
            Serial.println("Frame to rgb888 failed...");
        }
        //print_whole_frame(temp_buffer); 
        downsize(temp_buffer, WIDTH*HEIGHT*3);
        print_frame(rgb_frame);
        delay(1000);
    }
    //*/

    //raw = !digitalRead(12);

    //mil_s = millis();

    camera_fb_t  * fb = esp_camera_fb_get();

    //mil_1 = millis();

    fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, temp_buffer);

    //mil_2 = millis();

    esp_camera_fb_return(fb);

    //mil_3 = millis();

    downsize(temp_buffer, WIDTH*HEIGHT*3);

    //mil_4 = millis();

    if (digitalRead(12))
        print_frame(rgb_frame);

    Serial.println(line_recogn(rgb_frame));

    //Serial.printf("\nTotal: %3d, s-1: %3d, 1-2: %3d, 2-3: %3d, 3-4: %3d\n\n", mil_4 - mil_s, mil_1 - mil_s, mil_2 - mil_1, mil_3 - mil_2, mil_4 - mil_3);

    delay(1000);
}
