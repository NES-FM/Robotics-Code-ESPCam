#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(4, 13, NEO_GRB + NEO_KHZ800);

void setup()
{
    pixels.begin();
  pixels.show();

  pixels.fill(pixels.Color(255, 255, 255));
  pixels.show();
}

void loop()
{
    
}