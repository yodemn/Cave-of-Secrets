#include <stdint.h>
#include "img.h"

#ifndef Backgrounds_H
#define Backgrounds_H

extern const uint16_t background0[160 * 128];
extern ImageData background0_img;
extern ImageData bg_img;

class Background {
    public: 
        int x;
        int y;
        void Draw();
        ImageData image;
        Background(int, int, int, ImageData);
    private:
        int imageNum;
};

#endif
