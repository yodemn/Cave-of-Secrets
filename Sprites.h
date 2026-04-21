//create sprite images and attributes 

#include <stdint.h>
#include "img.h"
#ifndef Sprites_H
#define Sprites_H

#define PLAYER_SPRITE_WIDTH 18
#define PLAYER_SPRITE_HEIGHT 23
#define PLAYER_IDLE_FRAME_COUNT 4
#define PLAYER_WALK_FRAME_COUNT 8
#define PLAYER_CHROMA_KEY 0x07E0
#define PLAYER_MIN_X 0
#define PLAYER_MAX_X (160 - PLAYER_SPRITE_WIDTH)
#define PLAYER_RUN_SPEED 3

extern const ImageData PlayerIdleFrames[PLAYER_IDLE_FRAME_COUNT];
extern const ImageData PlayerWalkFrames[PLAYER_WALK_FRAME_COUNT];

class AnimatedPlayer{
    public:
        int x;
        int y;
        bool walking;
        bool facingLeft;
        uint8_t frame;
        uint8_t tick;

        AnimatedPlayer(int startX, int startY);
        void SetWalking(bool isWalking);
        void SetFacingLeft(bool left);
        void Move(int dx);
        void Update();
        void Draw();
        ImageData CurrentImage();
};

//image of platform 
const uint16_t platform[] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF
};


#endif
