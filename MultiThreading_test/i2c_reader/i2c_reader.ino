#include <Wire.h>
 
void setup() {
  Wire.begin(14, 15, 8000000);
  Serial.begin(115200);
  Serial.println("\nRead I2C\n");
}

String i2c_read_register(int DEVICE_I2C_ADDRESS, int DEVICE_REGISTER, int NUM_BYTES)
{
    Wire.beginTransmission(DEVICE_I2C_ADDRESS); // select device with "beginTransmission()"
    Wire.write(DEVICE_REGISTER); // select starting register with "write()"
    Wire.endTransmission(); // end write operation, as we just wanted to select the starting register
    Wire.requestFrom(DEVICE_I2C_ADDRESS, NUM_BYTES); // select number of bytes to get from the device (2 bytes in this case)

    String out;

    while(Wire.available())    // slave may send less than requested
    { 
        byte b = Wire.read(); // receive a byte as character
        out += b;
    }
}
 
void loop()
{
    Serial.println(i2c_read_register(0x20, 0x20, 5));
}