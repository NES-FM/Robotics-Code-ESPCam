#include <Wire.h>
 
int x = 0;
char* s = "asdf";

void setup() {
  Wire.begin(2, 4, 400000);
  Serial.begin(115200);
  Serial.println("\nWrite on I2C\n");
}

void i2c_write_register(int DEVICE_I2C_ADDRESS, int DEVICE_REGISTER, char* toWrite)
{
    Serial.println("Before begin");
    Wire.beginTransmission(DEVICE_I2C_ADDRESS); // select device with "beginTransmission()"
    Serial.println("Before write1");
    Wire.write(DEVICE_REGISTER); // select starting register with "write()"
    Serial.println("Before write2");
    Wire.write(toWrite);
    Serial.println("Before end");
    Wire.endTransmission(); // end write operation, as we just wanted to select the starting register
}
 
void loop()
{
    Serial.println("Before sprintf");
    Serial.println("Before func");
    i2c_write_register(64, 0x20, s);
    Serial.println("Before x++");
    x++;
    Serial.println("Before Delay");
    delay(1000);
}
