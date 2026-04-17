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
    private:
        int imageNum;
        
    Background(int, int, int, ImageData);
};

#endif