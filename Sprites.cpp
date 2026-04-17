#include <stdint.h>
#include "Sprites.h"
#include  "../inc/ST7735.h"


//main player sprite
const uint16_t player_pix[] = {
    0xFFFF, 0XFFFF, 0XFFFF, 0XFFFF,0xFFFF, 0XFFFF, 0XFFFF, 0XFFFF,0xFFFF, 0XFFFF, 0XFFFF, 0XFFFF,0xFFFF, 0XFFFF, 0XFFFF, 0XFFFF
};

ImageData player_img = {
    player_pix,
    10,
    30
};

Sprite::Sprite(int startX, int startY, ImageData img){
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