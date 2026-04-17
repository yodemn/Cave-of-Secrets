#ifndef Draw_H
#define Draw_H

#include <stdint.h>

struct ImageData {
    const uint16_t* pixels; // pointer to array for image
    uint8_t width;
    uint8_t height;
};
#endif 
