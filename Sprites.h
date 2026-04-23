#ifndef SPRITES_H
#define SPRITES_H
#include "img.h"
#include <stdint.h>
#define PLAYER_SPRITE_WIDTH 14
#define PLAYER_SPRITE_HEIGHT 18
#define PLAYER_IDLE_FRAME_COUNT 4
#define PLAYER_WALK_FRAME_COUNT 8
#define PLAYER_CHROMA_KEY 0x07E0
#define PLAYER_MIN_X 0
#define PLAYER_MAX_X (160 - PLAYER_SPRITE_WIDTH)
#define PLAYER_RUN_SPEED 3
#define PLAYER_JUMP_VELOCITY -8

extern const ImageData PlayerIdleFrames[PLAYER_IDLE_FRAME_COUNT];
extern const ImageData PlayerWalkFrames[PLAYER_WALK_FRAME_COUNT];


class AnimatedPlayer {
public:
    int x;
    int y;
    bool walking;
    bool facingLeft;
    uint8_t frame;
    uint8_t tick;

    // --- Physics & Jump Variables ---
    int velocityY;
    int gravity;
    bool isGrounded;
    bool wasJumpHeldLastFrame;
    int jumpTimer;
    
    // Set your max jump time limit right here!
    static const int MAX_JUMP_TIME = 15; 
    
    // --- Combo Variables ---
    int comboState;
    int comboTimer;
    bool wasJoySelectHeldLastFrame;

    // --- Functions ---
    AnimatedPlayer(int startX, int startY);

    void UpdatePhysics();
    void LandOn(int groundY);
    void StartFalling();
    
    // Notice how this matches the new CPP fix!
    void Jump(bool jumpButtonHeld, bool joySelectHeld); 
    
    void SetWalking(bool isWalking);
    void SetFacingLeft(bool left);
    void Move(int dx);
    void Update();
    ImageData CurrentImage();
    void Draw();
};

#endif
