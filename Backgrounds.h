#include <stdint.h>
#include "img.h"

#ifndef Backgrounds_H
#define Backgrounds_H

struct BackgroundColorRow {
    uint16_t color;
    uint8_t length;
};

extern ImageData background0_img;
extern ImageData bg_img;

void drawRowsOfColorForBackground(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

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
