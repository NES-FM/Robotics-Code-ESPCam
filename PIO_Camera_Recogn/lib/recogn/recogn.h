#ifndef RECOGN_H
#define RECOGN_H

String line = "";

float rad_to_deg(float rad)
{
    return ((rad * 4068) / 71);
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
            if (frame[y][x][0] == 0 && frame[y][x][1] == 0 && frame[y][x][2] == 0)
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
            if (frame[y][x][0] == 0 && frame[y][x][1] == 0 && frame[y][x][2] == 0)
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

    // Only measure certain things if the line is centered enough
    distance_from_top = (measured_top_left[1] > RECOGN_WAIT_THRESHOLD || measured_top_right[1] > RECOGN_WAIT_THRESHOLD);
    distance_from_bottom = (measured_bottom_left[1] <= RECOGN_WAIT_THRESHOLD_BOTTOM || measured_bottom_right[1] <= RECOGN_WAIT_THRESHOLD_BOTTOM);

    // Standard Ltype, angle, midfactor stuff
    if (distance_from_top && distance_from_bottom)
    {
        // iterating from Top to Bottom
        for (int y = 0; y < END_RESOLUTION; y++)
        {
            // iterating from left to right
            for (int x = 0; x < END_RESOLUTION; x++)
            {
                if (frame[y][x][0] == 0 && frame[y][x][1] == 0 && frame[y][x][2] == 0)
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
                if (frame[y][x][0] == 0 && frame[y][x][1] == 0 && frame[y][x][2] == 0)
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
    cuart_set_line(ltype, angle, midfactor);

    // SENSOR ARRAY
    unsigned char temp_cuart_sensor_array[24] = {0};

    // Two Bits: 1st signalling the crossing is centered, 2nd showing that a space is incomming, but not completely white yet
    temp_cuart_sensor_array[0] = !(measured_top_left[1] > (RECOGN_WAIT_THRESHOLD+2) || measured_top_right[1] > (RECOGN_WAIT_THRESHOLD+2));
    temp_cuart_sensor_array[1] = !(measured_top_left[1] > RECOGN_SPACE_THRESHOLD && measured_top_right[1] > RECOGN_SPACE_THRESHOLD);

    if(debug) Serial.printf("%d ", temp_cuart_sensor_array[0]);
    if(debug) Serial.printf("%d ", temp_cuart_sensor_array[1]);
    
    // Real Sensor array
    for(int x = 2; x < END_RESOLUTION; x++)
    {
        int red = frame[OFFSET_SENSOR_ARRAY][x][0];
        int green = frame[OFFSET_SENSOR_ARRAY][x][1];
        int blue = frame[OFFSET_SENSOR_ARRAY][x][2];
        if (red == 0 && green == 0 && blue == 0)
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
    }

    // Green Recognition
    if (distance_from_top && distance_from_bottom)
    {
        // bool first_green = false;
        // bool second_green = false;
        // bool first_black = false;
        // bool second_black = false;

        int tl = 0;
        int tr = 0;
        int bl = 0;
        int br = 0;

        for (int x = 0; x < END_RESOLUTION; x++)
        {
            for (int y = END_RESOLUTION-1; y >= 0; y--)
            {
                if (frame[y][x][0] == 0 && frame[y][x][1] == 255 && frame[y][x][2] == 0)
                {
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
                }
            }
        }
        if (debug) Serial.printf("  TL: %d, TR: %d, BR: %d, BL: %d\r\n", tl, tr, br, bl);
        cuart_set_green((tl >= 10), (tr >= 10), (bl >= 10), (br >= 10));
    }
    else
    {
        cuart_set_green(false, false, false, false);
    }

    return true;
}           

#endif