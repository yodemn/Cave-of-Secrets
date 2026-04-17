#include <stdint.h>
#include "Backgrounds.h"
#include  "../inc/ST7735.h"

Background::Background() {
    x = startX;
    y = startY;
    velocityY = 0;
    image = img;
}

void Sprite::Move(){
    y += velocityY;

}

void Sprite::Draw(){
    ST7735_DrawBitmap(x, y, image.pixels, image.width, image.height);
}