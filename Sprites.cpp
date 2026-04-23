#include <stdint.h>
#include "Sprites.h"
#include "../inc/ST7735.h"
#include "img.h"
#include "PlayerSpritesData.inc"

static void DrawPlayerImage(int16_t x, int16_t y, ImageData image, bool flipX){
    for(int16_t row = 0; row < image.height; row++){
        int16_t screenY = y - image.height + 1 + row;
        int16_t sourceRow = image.height - 1 - row;
        for(int16_t col = 0; col < image.width; col++){
            int16_t sourceCol = flipX ? (image.width - 1 - col) : col;
            uint16_t color = image.pixels[sourceRow * image.width + sourceCol];
            if(color != PLAYER_CHROMA_KEY){
                ST7735_DrawPixel(x + col, screenY, color);
            }
        }
    }
}

AnimatedPlayer::AnimatedPlayer(int startX, int startY){
    x = startX;
    y = startY;
    walking = false;
    facingLeft = false;
    frame = 0;
    tick = 0;
    velocityY = 0;
    gravity = 1;
    isGrounded = true;
    wasJumpHeldLastFrame = false;
    comboState = 0;
    comboTimer = 0;
    wasJoySelectHeldLastFrame = false;

}

void AnimatedPlayer::UpdatePhysics(){
    if(!isGrounded){
        velocityY += gravity; 
    }
    y += velocityY;
    
    // Ceiling check
    if(y < 16){ 
        y = 16;          
        velocityY = 0;   
    }

    // Floor check
    if(y >= 127){ 
        y = 127;           
        velocityY = 0;     
        isGrounded = true; 
        
        // --- NEW: Reset Combo on Landing ---
        comboState = 0;
        gravity = 1; // Failsafe to ensure gravity is on
    }
}

void AnimatedPlayer::LandOn(int groundY){
    y = groundY;
    velocityY = 0;
    isGrounded = true;
}

void AnimatedPlayer::StartFalling(){
    if(isGrounded){
        isGrounded = false;
        velocityY = 0;
    }
}

void AnimatedPlayer::Jump(bool jumpButtonHeld, bool joySelectHeld){
    // --- 1. THE COMBO TIMER ---
    if(comboState > 0){
        comboTimer++;
        // If 1 second (30 frames) passes, combo fails
        if(comboTimer > 30){
            comboState = 0; 
            gravity = 1;     
        }
    }

    // --- 2. STEP 1: INITIAL JUMP ---
    if(isGrounded && jumpButtonHeld && !wasJumpHeldLastFrame){
        isGrounded = false;
        
        // Apply one single burst of upward speed
        velocityY = PLAYER_JUMP_VELOCITY+1;  
        
        comboState = 1;
        comboTimer = 0;
    }

    // --- 3. STEP 2: MID-AIR PAUSE ---
    if(!isGrounded && comboState < 2 && joySelectHeld && !wasJoySelectHeldLastFrame){
        comboState = 2; 
        comboTimer = 0; 
        velocityY = 0;   
        gravity = 0;     // Hover!
    }

    // --- 4. STEP 3: DOUBLE JUMP ---
    if(!isGrounded && comboState == 2 && jumpButtonHeld && !wasJumpHeldLastFrame){
        comboState = 3;  
        gravity = 1;     
        
        // Give them the exact same medium burst of speed for the double jump
        velocityY = PLAYER_JUMP_VELOCITY+2;  
    }

    // Save states so they can't just hold the button down!
    wasJumpHeldLastFrame = jumpButtonHeld;
    wasJoySelectHeldLastFrame = joySelectHeld;
}
void AnimatedPlayer::SetWalking(bool isWalking){
    if(walking != isWalking){
        walking = isWalking;
        frame = 0;
        tick = 0;
    }
}

void AnimatedPlayer::SetFacingLeft(bool left){
    facingLeft = left;
}

void AnimatedPlayer::Move(int dx){
    x += dx;
    if(x < PLAYER_MIN_X){
        x = PLAYER_MIN_X;
    }
    if(x > PLAYER_MAX_X){
        x = PLAYER_MAX_X;
    }
}

void AnimatedPlayer::Update(){
    uint8_t frameCount = walking ? PLAYER_WALK_FRAME_COUNT : PLAYER_IDLE_FRAME_COUNT;
    uint8_t frameDelay = walking ? 3 : 8;

    tick++;
    if(tick >= frameDelay){
        tick = 0;
        frame++;
        if(frame >= frameCount){
            frame = 0;
        }
    }
}

ImageData AnimatedPlayer::CurrentImage(){
    if(walking){
        return PlayerWalkFrames[frame];
    }
    return PlayerIdleFrames[frame];
}

void AnimatedPlayer::Draw(){
    DrawPlayerImage(x, y, CurrentImage(), facingLeft);
}
