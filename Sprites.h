//create sprite images and attributes 

#include <stdint.h>
#include "img.h"
#ifndef Sprites_H
#define Sprites_H


class Sprite{
    public:
        int x;
        int y;
        int velocityY;
        ImageData image;

        //Constructor and functions
        Sprite(int startX, int startY, ImageData img);
        void Draw();
        void Move(); 
};

//image of platform 
const uint16_t platform[] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF
};


#endif