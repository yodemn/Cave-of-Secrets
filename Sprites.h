//create sprite images and attributes 

#include <stdint.h>
#include "img.h"
#ifndef Sprites_H
#define Sprites_H

#define PLAYER_SPRITE_WIDTH 16
#define PLAYER_SPRITE_HEIGHT 20
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
        int velocityY;
        int gravity;
        bool isGrounded;
        int jumpTimer;
        const int MAX_JUMP_TIME = 15;
        bool wasJumpHeldLastFrame;
        AnimatedPlayer(int startX, int startY);
        void SetWalking(bool isWalking);
        void Jump(bool buttonHeld);
        void UpdatePhysics();
        void LandOn(int groundY);
        void StartFalling();
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
