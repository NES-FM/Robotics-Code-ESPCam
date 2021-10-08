TaskHandle_t cuart_task;

bool cuart_line_changed = false;
int cuart_line_type = 0;
signed char cuart_line_angle = 0x00;
signed char cuart_line_midfactor = 0x00;

bool cuart_green_changed = false;
unsigned char cuart_green = 0b0000;

const int cuart_ltype_straight = 0x00;
const int cuart_ltype_90l = 0x01;
const int cuart_ltype_90r = 0x02;
const int cuart_ltype_tl = 0x03;
const int cuart_ltype_tr = 0x04;
const int cuart_ltype_t = 0x05;
const int cuart_ltype_X = 0x06;
const int cuart_ltype_space = 0x07;
const int cuart_ltype_unknown = 0x20;

unsigned char cuart_sensor_array[3] = {0x00, 0x00, 0x00};
bool cuart_sensor_array_changed = false;

void cuart_set_line(int type, int angle, int midfactor)
{
	cuart_line_type = type;
	cuart_line_angle = (signed char)angle;
	cuart_line_midfactor = (signed char)midfactor;
	cuart_line_changed = true;
}

void cuart_set_sensor_array(unsigned char param[24])
{
	unsigned char tmp = 0x00;
	for (int i = 0; i < 8; i++)
	{
		tmp <<= 1;
		tmp |= param[i];
	}
	cuart_sensor_array[0] = tmp;
	tmp = 0x00;
	for (int i = 8; i < 16; i++)
	{
		tmp <<= 1;
		tmp |= param[i];
	}
	cuart_sensor_array[1] = tmp;
	tmp = 0x00;
	for (int i = 16; i < 24; i++)
	{
		tmp <<= 1;
		tmp |= param[i];
	}
	cuart_sensor_array[2] = tmp;
	cuart_sensor_array_changed = true;
}

void cuart_set_green(bool TL, bool TR, bool DL, bool DR)
{
	cuart_green = ((TL << 3) |(TR << 2) |(DL << 1) |(DR));
	cuart_green_changed = true;
}

void cuart_code(void* parameter)
{
    Serial.print("Cuart running on core ");
    Serial.println(xPortGetCoreID());
    delay(500);
    while (true) {
        if (cuart_line_changed)
        {
            //Serial.println("Send Cuart Line");
        	cuart_line_changed = false;
		    Serial1.write("L");
		    Serial1.write(cuart_line_type);
		    Serial1.write(cuart_line_angle);
		    Serial1.write(cuart_line_midfactor);
		    Serial1.write(0x00);
			Serial1.write(0xFF);
		}
		if (cuart_green_changed)
		{
			cuart_green_changed = false;
		    Serial1.write("G");
		    Serial1.write(cuart_green);
		    Serial1.write(0x00);
			Serial1.write(0xFF);		
		}
		if (cuart_sensor_array_changed)
		{
			cuart_sensor_array_changed = false;
			Serial1.write('S');
			Serial1.write(cuart_sensor_array[0]);
			Serial1.write(cuart_sensor_array[1]);
			Serial1.write(cuart_sensor_array[2]);
			Serial1.write(0x00);
			Serial1.write(0xFF);	
		}
    }
}

void cuart_init()
{
    Serial1.begin(115200, SERIAL_8N1, 14, 15);
    xTaskCreatePinnedToCore(cuart_code, /* Function to implement the task */
        "Cuart", /* Name of the task */
        10000, /* Stack size in words */
        NULL, /* Task input parameter */
        0, /* Priority of the task */
        &cuart_task, /* Task handle. */
        0); /* Core where the task should run */
}
