#ifndef RECOGN_H
#define RECOGN_H

String line = "";

float rad_to_deg(float rad)
{
    return ((rad * 4068) / 71);
}

bool green_is_point_left_of_line(int point_x, int point_y, int top_mid_x, int top_mid_y, int bottom_mid_x, int bottom_mid_y)
{
    //Steigung:
    float t = (bottom_mid_x - top_mid_x);
    float s = (bottom_mid_y - top_mid_y);
    float m = (t / s);
    //Achsenabschnitt:
    float c = top_mid_x - (m * top_mid_y);
    // if (debug)Serial.printf("Is line left? P(%d|%d) TopMid(%d|%d) BottomMid(%d|%d) c:%f m:%f\r\n", point_x, point_y, top_mid_x, top_mid_y, bottom_mid_x, bottom_mid_y, c, m);
    return point_x < (m * point_y + c);
}

bool line_recogn(uint8_t frame[END_RESOLUTION][END_RESOLUTION][3])
{
    int measured_left_top[2] = {0, 0};    // red
    int measured_left_bottom[2] = {0, 0}; // red
    bool left_top_found = false;

    int measured_right_top[2] = {0, 0};    // blue
    int measured_right_bottom[2] = {0, 0}; // blue
    bool right_top_found = false;

    int measured_top_left[2] = {0, 0};  // green
    int measured_top_right[2] = {0, 0}; // green
    bool top_left_found = false;

    int measured_bottom_left[2] = {0, 0};  // pink
    int measured_bottom_right[2] = {0, 0}; // pink
    bool bottom_left_found = false;

    // bool flag_wait = false;

    bool top_left_equal = false;     // red + green
    bool top_right_equal = false;    // blue + green
    bool bottom_left_equal = false;  // red + pink
    bool bottom_right_equal = false; // blue + pink

    int ltype = cuart_ltype_straight;
    int angle = 0;
    int midfactor = 0;

    int delta[2] = {0, 0};

    int left_angle = 0;
    int right_angle = 0;
    int top_angle = 0;
    int bottom_angle = 0;

    int top_midfactor = 0;
    int bottom_midfactor = 0;

    bool distance_from_top = false;
    bool distance_from_bottom = false;

    // iterating from Left to Right
    for (int x = 0; x < END_RESOLUTION; x++)
    {
        // iterating from Top to Bottom
        for (int y = 0; y < END_RESOLUTION; y++)
        {
            if (is_pixel_black_or_silver(x, y))
            {
                if (top_left_found)
                {
                    measured_top_right[0] = x;
                    measured_top_right[1] = y;
                }
                else
                {
                    measured_top_left[0] = x;
                    measured_top_left[1] = y;
                    top_left_found = true;
                }
                break;
            }
        }
        // iterating from right to left
        for (int y = END_RESOLUTION - 1; y >= 0; y--)
        {
            if (is_pixel_black_or_silver(x, y))
            {
                if (bottom_left_found)
                {
                    measured_bottom_right[0] = x;
                    measured_bottom_right[1] = y;
                }
                else
                {
                    measured_bottom_left[0] = x;
                    measured_bottom_left[1] = y;
                    bottom_left_found = true;
                }
                break;
            }
        }
    }

    // iterating from Top to Bottom
    for (int y = 0; y < END_RESOLUTION; y++)
        {
            // iterating from left to right
            for (int x = 0; x < END_RESOLUTION; x++)
            {
                if (is_pixel_black_or_silver(x, y))
                {
                    if (left_top_found)
                    {
                        measured_left_bottom[0] = x;
                        measured_left_bottom[1] = y;
                    }
                    else
                    {
                        measured_left_top[0] = x;
                        measured_left_top[1] = y;
                        left_top_found = true;
                    }
                    break;
                }
            }
            // iterating from right to left
            for (int x = END_RESOLUTION - 1; x >= 0; x--)
            {
                if (is_pixel_black_or_silver(x, y))
                {
                    if (right_top_found)
                    {
                        measured_right_bottom[0] = x;
                        measured_right_bottom[1] = y;
                    }
                    else
                    {
                        measured_right_top[0] = x;
                        measured_right_top[1] = y;
                        right_top_found = true;
                    }
                    break;
                }
            }
        }

    int green_left_mid_coordinate = int((measured_top_left[1] + measured_bottom_left[1]) / 2); // y
    int green_right_mid_coordinate = int((measured_top_right[1] + measured_bottom_right[1]) / 2); // y
    int green_top_mid_coordinate = int((measured_left_top[0] + measured_right_top[0]) / 2); // x
    int green_bottom_mid_coordinate = int((measured_left_bottom[0] + measured_right_bottom[0]) / 2); // x

    // Only measure certain things if the line is centered enough
    /*
    distance_from_top = (measured_top_left[1] > RECOGN_WAIT_THRESHOLD || measured_top_right[1] > RECOGN_WAIT_THRESHOLD);
    distance_from_bottom = (measured_bottom_left[1] <= RECOGN_WAIT_THRESHOLD_BOTTOM || measured_bottom_right[1] <= RECOGN_WAIT_THRESHOLD_BOTTOM);
    //*/
    ///*
    distance_from_top = ((green_left_mid_coordinate > RECOGN_WAIT_THRESHOLD) || (green_right_mid_coordinate > RECOGN_WAIT_THRESHOLD));  //((green_left_mid_coordinate + green_right_mid_coordinate) / 2) > RECOGN_WAIT_THRESHOLD;
    distance_from_bottom = ((green_left_mid_coordinate < RECOGN_WAIT_THRESHOLD_BOTTOM) || (green_right_mid_coordinate < RECOGN_WAIT_THRESHOLD_BOTTOM));  //((green_left_mid_coordinate + green_right_mid_coordinate) / 2) < RECOGN_WAIT_THRESHOLD_BOTTOM;
    //*/

    // Standard Ltype, angle, midfactor stuff
    if (distance_from_top && distance_from_bottom)
    {
        if (abs(measured_left_top[0] - measured_top_left[0]) <= RECOGN_MAX_OFFSET || abs(measured_left_top[1] - measured_top_left[1]) <= RECOGN_MAX_OFFSET)
        {
            top_left_equal = true;
        }
        if (abs(measured_right_top[0] - measured_top_right[0]) <= RECOGN_MAX_OFFSET || abs(measured_right_top[1] - measured_top_right[1]) <= RECOGN_MAX_OFFSET)
        {
            top_right_equal = true;
        }
        if (abs(measured_left_bottom[0] - measured_bottom_left[0]) <= RECOGN_MAX_OFFSET || abs(measured_left_bottom[1] - measured_bottom_left[1]) <= RECOGN_MAX_OFFSET)
        {
            bottom_left_equal = true;
        }
        if (abs(measured_right_bottom[0] - measured_bottom_right[0]) <= RECOGN_MAX_OFFSET || abs(measured_right_bottom[1] - measured_bottom_right[1]) <= RECOGN_MAX_OFFSET)
        {
            bottom_right_equal = true;
        }

        // ... And by doing so getting the ltype
        if ((top_left_equal && top_right_equal && bottom_left_equal && bottom_right_equal) /*|| (!top_left_equal && top_right_equal && bottom_left_equal && !bottom_right_equal) || (top_left_equal && !top_right_equal && !bottom_left_equal && bottom_right_equal)*/)
        {
            ltype = cuart_ltype_straight;
        }
        else if (top_left_equal && top_right_equal && !bottom_left_equal && bottom_right_equal)
        {
            ltype = cuart_ltype_90l;
        }
        else if (top_left_equal && top_right_equal && bottom_left_equal && !bottom_right_equal)
        {
            ltype = cuart_ltype_90r;
        }
        else if (!top_left_equal && top_right_equal && !bottom_left_equal && bottom_right_equal)
        {
            ltype = cuart_ltype_tl;
        }
        else if (top_left_equal && !top_right_equal && bottom_left_equal && !bottom_right_equal)
        {
            ltype = cuart_ltype_tr;
        }
        else if (top_left_equal && top_right_equal && !bottom_left_equal && !bottom_right_equal)
        {
            ltype = cuart_ltype_t;
        }
        else if (!top_left_equal && !top_right_equal && !bottom_left_equal && !bottom_right_equal)
        {
            ltype = cuart_ltype_X;
        }
        else
        {
            ltype = cuart_ltype_unknown;
        }

        // red
        delta[0] = measured_left_bottom[0] - measured_left_top[0];
        delta[1] = measured_left_bottom[1] - measured_left_top[1];
        left_angle = constrain(round(rad_to_deg(atan2(delta[1], delta[0]))) - 90, -90, 90);
        // blue
        delta[0] = measured_right_bottom[0] - measured_right_top[0];
        delta[1] = measured_right_bottom[1] - measured_right_top[1];
        right_angle = constrain(round(rad_to_deg(atan2(delta[1], delta[0]))) - 90, -90, 90);
        // green
        delta[0] = measured_top_right[0] - measured_top_left[0];
        delta[1] = measured_top_right[1] - measured_top_left[1];
        top_angle = constrain(round(rad_to_deg(atan2(delta[1], delta[0]))) - 90, -90, 90);
        // pink
        delta[0] = measured_bottom_right[0] - measured_bottom_left[0];
        delta[1] = measured_bottom_right[1] - measured_bottom_left[1];
        bottom_angle = constrain(round(rad_to_deg(atan2(delta[1], delta[0]))) - 90, -90, 90);

        // Calculating the Angle based on the ltype
        if (ltype == cuart_ltype_straight || ltype == cuart_ltype_tl || ltype == cuart_ltype_tr || ltype == cuart_ltype_X)
        { // average between left and right angles
            angle = (left_angle + right_angle) / 2;
        }
        else if (ltype == cuart_ltype_90l)
        { // right angle
            angle = right_angle;
        }
        else if (ltype == cuart_ltype_90r)
        { // left angle
            angle = left_angle;
        }
        else if (ltype == cuart_ltype_t)
        { // average between top and bottom
            angle = (top_angle + bottom_angle) / 2;
        }

        // The deviation from the center value | too left means negative
        //                     (mid of line                                       ) - (mid of image          )
        top_midfactor = int(int((measured_left_top[0] + measured_right_top[0]) / 2) - ((END_RESOLUTION - 1) / 2));
        bottom_midfactor = int(int((measured_left_bottom[0] + measured_right_bottom[0]) / 2) - ((END_RESOLUTION - 1) / 2));

        // Calculating the midfactor based on the ltype
        if (ltype == cuart_ltype_straight || ltype == cuart_ltype_tl || ltype == cuart_ltype_tr || ltype == cuart_ltype_X || ltype == cuart_ltype_unknown)
        { // average between top and bottom
            midfactor = (bottom_midfactor + top_midfactor) / 2;
        }
        else if (ltype == cuart_ltype_90l || ltype == cuart_ltype_90r || ltype == cuart_ltype_t)
        { //bottom midfactor
            midfactor = bottom_midfactor;
        }

        if (measured_top_left[1] > RECOGN_SPACE_THRESHOLD && measured_top_right[1] > RECOGN_SPACE_THRESHOLD)
            ltype = cuart_ltype_space;
    }
    else
    {
        // red
        delta[0] = measured_left_bottom[0] - measured_left_top[0];
        delta[1] = measured_left_bottom[1] - measured_left_top[1];
        left_angle = constrain(round(rad_to_deg(atan2(delta[1], delta[0]))) - 90, -90, 90);
        // blue
        delta[0] = measured_right_bottom[0] - measured_right_top[0];
        delta[1] = measured_right_bottom[1] - measured_right_top[1];
        right_angle = constrain(round(rad_to_deg(atan2(delta[1], delta[0]))) - 90, -90, 90);

        top_midfactor = int(int((measured_left_top[0] + measured_right_top[0]) / 2) - ((END_RESOLUTION - 1) / 2));
        bottom_midfactor = int(int((measured_left_bottom[0] + measured_right_bottom[0]) / 2) - ((END_RESOLUTION - 1) / 2));

        ltype = cuart_ltype_unknown;
        angle = (left_angle + right_angle) / 2;
        midfactor = (top_midfactor + bottom_midfactor) / 2;
    }
    cuart_set_line(ltype, angle, midfactor);

    // SENSOR ARRAY
    unsigned char temp_cuart_sensor_array[24] = {0};

    // Two Bits: 1st signalling the crossing is centered, 2nd showing that a space is incomming, but not completely white yet
    temp_cuart_sensor_array[0] = !((measured_top_left[1] > RECOGN_WAIT_THRESHOLD && measured_bottom_left[1] < RECOGN_WAIT_THRESHOLD_BOTTOM) || (measured_top_right[1] > RECOGN_WAIT_THRESHOLD && measured_bottom_right[1] < RECOGN_WAIT_THRESHOLD_BOTTOM));//!(distance_from_top && distance_from_bottom);//!(measured_top_left[1] > (RECOGN_WAIT_THRESHOLD+2) || measured_top_right[1] > (RECOGN_WAIT_THRESHOLD+2));
    temp_cuart_sensor_array[1] = true;//!(measured_top_left[1] > RECOGN_SPACE_THRESHOLD && measured_top_right[1] > RECOGN_SPACE_THRESHOLD);

    if(debug) Serial.printf("%d ", temp_cuart_sensor_array[0]);
    if(debug) Serial.printf("%d ", temp_cuart_sensor_array[1]);
    
    // Real Sensor array
    if(debug) Serial.print("Array: ");
    for(int x = 2; x < END_RESOLUTION; x++)
    {
        if (is_pixel_black_or_silver(x, OFFSET_SENSOR_ARRAY))
        {
            temp_cuart_sensor_array[x] = 0;
        }
        else
        {
            temp_cuart_sensor_array[x] = 1;
        }

        // if (red > 0 || green > 0 || blue > 0)
        //     temp_cuart_sensor_array[x] = 1;
        // else
        //     temp_cuart_sensor_array[x] = 0;

        if(debug) Serial.printf("%d ", temp_cuart_sensor_array[x]); 
    }
    cuart_set_sensor_array(temp_cuart_sensor_array);

    if (debug)
    {
        Serial.println("");
        Serial.println("");
        Serial.print("Ltype: ");
        Serial.print(ltype);
        Serial.print("  Angle: ");
        Serial.print(angle);
        Serial.print("  Midfactor: ");
        Serial.print(midfactor);
        Serial.println("");
        Serial.print("TL equal: ");
        Serial.print(top_left_equal);
        Serial.print(" TR equal: ");
        Serial.print(top_right_equal);
        Serial.print(" BL equal: ");
        Serial.print(bottom_left_equal);
        Serial.print(" BR equal: ");
        Serial.println(bottom_left_equal);
        Serial.print("(Red)Measured Left Top: ");
        Serial.print(measured_left_top[0]);
        Serial.print("|");
        Serial.print(measured_left_top[1]);
        Serial.print(" Measured Left Bottom: ");
        Serial.print(measured_left_bottom[0]);
        Serial.print("|");
        Serial.println(measured_left_bottom[1]);
        Serial.print("(Blue)Measured Right Top: ");
        Serial.print(measured_right_top[0]);
        Serial.print("|");
        Serial.print(measured_right_top[1]);
        Serial.print(" Measured Right Bottom: ");
        Serial.print(measured_right_bottom[0]);
        Serial.print("|");
        Serial.println(measured_right_bottom[1]);
        Serial.print("(Green)Measured Top Left: ");
        Serial.print(measured_top_left[0]);
        Serial.print("|");
        Serial.print(measured_top_left[1]);
        Serial.print(" Measured Top Right: ");
        Serial.print(measured_top_right[0]);
        Serial.print("|");
        Serial.println(measured_top_right[1]);
        Serial.print("(Pink)Measured Bottom Left: ");
        Serial.print(measured_bottom_left[0]);
        Serial.print("|");
        Serial.print(measured_bottom_left[1]);
        Serial.print(" Measured Bottom Right: ");
        Serial.print(measured_bottom_right[0]);
        Serial.print("|");
        Serial.println(measured_bottom_right[1]);
        Serial.println();

        Serial.printf("Top Mid: %d, Bot Mid: %d, Left Mid: %d, Right Mid: %d\r\n", green_top_mid_coordinate, green_bottom_mid_coordinate, green_left_mid_coordinate, green_right_mid_coordinate);
        Serial.println();
    }

    // Green, Red, Silver Recognition
    if (true)//distance_from_top && distance_from_bottom)
    {
        // bool first_green = false;
        // bool second_green = false;
        // bool first_black = false;
        // bool second_black = false;

        int tl = 0;
        int tr = 0;
        int bl = 0;
        int br = 0;

        int red = 0;

        int silver = 0;

        for (int y = 0; y < END_RESOLUTION; y++)
        {
            for (int x = 0; x < END_RESOLUTION; x++)
            {
                if (is_pixel_green(x, y))
                {
                    /*
                    if (y > green_left_mid_coordinate && x < green_bottom_mid_coordinate)
                    {
                        bl += 1;
                    }
                    else if (y > green_right_mid_coordinate && x > green_bottom_mid_coordinate)
                    {
                        br += 1;
                    }
                    else if (y < green_left_mid_coordinate && x < green_top_mid_coordinate)
                    {
                        tl += 1;
                    }
                    else if (y < green_right_mid_coordinate && x > green_top_mid_coordinate)
                    {
                        tr += 1;
                    }*/
                    
                    bool is_left = false;

                    // In Case of T Crossing / Circle: fall back to using only bottom
                    if (measured_left_top[1] > 5 || measured_right_top[1] > 5)
                    {
                        is_left = x < green_bottom_mid_coordinate;
                    }
                    else
                    {
                        is_left = green_is_point_left_of_line(x, y, green_top_mid_coordinate, 1, green_bottom_mid_coordinate, 23);
                    } 

                    if (is_left)
                    {
                        if (y > green_left_mid_coordinate)
                        {
                            bl += 1;
                        }
                        else
                        {
                            tl += 1;
                        }
                    }
                    else
                    {
                        if (y > green_right_mid_coordinate)
                        {
                            br += 1;
                        }
                        else
                        {
                            tr += 1;
                        }
                    }

                    if (debug) Serial.printf("Green point at: %d|%d\r\n", x, y);
                    /*
                    if (y < (END_RESOLUTION / 2))
                    {
                        if (x > (END_RESOLUTION / 2))
                            tr += 1;
                        else
                            tl += 1;
                    }
                    else
                    {
                        if (x > (END_RESOLUTION / 2))
                            br += 1;
                        else
                            bl += 1;
                    }
                    if (debug) Serial.printf("Green point at: %d|%d. T: %s, R: %s", x, y, y < (END_RESOLUTION / 2) ? "T" : "F", x > (END_RESOLUTION / 2) ? "T" : "F");
                    //*/
                }
                else if (frame[y][x][0] == 255 && frame[y][x][1] == 0 && frame[y][x][2] == 0)
                {
                    red++;
                }
                else if (frame[y][x][0] == 128 && frame[y][x][1] == 128 && frame[y][x][2] == 128)
                {
                    silver++;
                }
            }
        }
        if (debug) Serial.printf("  TL: %d, TR: %d, BR: %d, BL: %d\r\n", tl, tr, br, bl);
        if (debug) Serial.printf("Red: %d, Silver: %d\r\n", red, silver);	
        cuart_set_green((tl >= 10), (tr >= 10), (bl >= 10), (br >= 10));
        if (red >= 60) cuart_set_red_line();
        if (silver >= 30) cuart_set_silver_line();
        if ((tl + tr + br + bl) >= 60) cuart_set_green_line();
    }
    else
    {
        cuart_set_green(false, false, false, false);
    }

    /* if (debug)
    {
        for (int y = 0; y < END_RESOLUTION; y++)
        {
            for (int x = 0; x < END_RESOLUTION; x++)
            {
                if (green_is_point_left_of_line(x, y, green_top_mid_coordinate, 1, green_bottom_mid_coordinate, 23))
                {
                    Serial.print("T");
                }
                else
                {
                    Serial.print("F");
                }
            }
            Serial.println("");
        }
    }*/

    return true;
}           

#endif
