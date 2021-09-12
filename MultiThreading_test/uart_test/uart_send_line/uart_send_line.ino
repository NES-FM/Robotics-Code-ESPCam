#include "cuart.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  cuart_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  cuart_set_line(cuart_ltype_straight, 13, 0);
  Serial.println("0 13 0");
  delay(2000);
  int i = 20;
  for (i = 20; i < 40; i++)
  {
    cuart_set_line(cuart_ltype_straight, i, 0);
    Serial.print("0 "); Serial.print(i); Serial.println(" 0");
    delay(500);
  }
}
