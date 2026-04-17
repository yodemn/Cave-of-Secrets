#include <stdint.h>
#include "Backgrounds.h"
#include  "../inc/ST7735.h"
#include "img.h"

Background::Background(int startX, int startY, int iNum, ImageData i) {
    x = startX;
    y = startY;
    imageNum = iNum;
    image = i;
} 


void Background::Draw(){
    ST7735_DrawBitmap(x, y, image.pixels, image.width, image.height);
}