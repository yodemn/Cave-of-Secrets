#include <stdint.h>
#include "img.h"

#ifndef Backgrounds_H
#define Backgrounds_H

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