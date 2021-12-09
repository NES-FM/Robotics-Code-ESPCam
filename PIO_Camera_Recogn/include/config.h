#ifndef CONFIG_H
#define CONFIG_H

// Defined by setting the Resolution
#define IMAGE_RESOLUTION FRAMESIZE_QQVGA
#define IMAGE_WIDTH 160
#define IMAGE_HEIGHT 120

// User Configureable stuff
#define OFFSET_RIGHT 18
// END_RESOLUTION needs to be a divisor of IMAGE_HEIGHT
#define END_RESOLUTION 24
// RECOGN_WAIT_THRESHOLD: Starting on which pixel should it start using all of the recognition algorithms
#define RECOGN_WAIT_THRESHOLD 8
// RECOGN_WAIT_THRESHOLD_BOTTOM: Until which pixel should it use all of the recognition algorithms
#define RECOGN_WAIT_THRESHOLD_BOTTOM 18
// RECOGN_SPACE_THRESHOLD: If the line is further than that, but the type
#define RECOGN_SPACE_THRESHOLD 16
// RECOGN_MAX_OFFSET = maximum difference between to measurements to be considered the same
#define RECOGN_MAX_OFFSET 2

// OFFSET_SENSOR_ARRAY is the offset from above to the sensor array
#define OFFSET_SENSOR_ARRAY 4

// PER_BLOCK is Calculated from the Values above.
#define PER_BLOCK (IMAGE_HEIGHT / END_RESOLUTION)

#endif