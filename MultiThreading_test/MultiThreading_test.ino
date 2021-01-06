#include <Wire.h>

TaskHandle_t Task1;

void setup() {
  Wire.begin(14, 15, 8000000);
  Serial.begin(115200);
  Serial.print("setup() running on core ");
  Serial.println(xPortGetCoreID());

  xTaskCreatePinnedToCore(
      Task1code, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &Task1,  /* Task handle. */
      0); /* Core where the task should run */
}

void loop() {
  Serial.print("loop() running on core ");
  Serial.println(xPortGetCoreID());
  while (true) { }
}

void Task1code( void * parameter) {
  while(true) {
    while(Wire.available())    // slave may send less than requested
    { 
        byte b = Wire.read(); // receive a byte as character
        Serial.print(b);
    }
    Serial.println();
  }
}
