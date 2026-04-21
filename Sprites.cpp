#include <stdint.h>
#include "Sprites.h"
#include "../inc/ST7735.h"

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
    jumpTimer = 0;
    wasJumpHeldLastFrame = false;
}

void AnimatedPlayer::UpdatePhysics(){
    // Always apply gravity if they are in the air
    if(!isGrounded){
        velocityY += gravity; 
    }

    // Move the player
    y += velocityY;

    // Floor Collision (Replace 100 with your actual floor Y-coordinate)
    if(y >= 128){ 
        y = 128;           // Snap to floor
        velocityY = 0;     // Stop falling
        isGrounded = true; // Ready to jump again
    }
}

void AnimatedPlayer::Jump(bool jumpButtonHeld){
    // 1. The Initial Press (Just tapped the button this frame)
    if(isGrounded && jumpButtonHeld && !wasJumpHeldLastFrame){
        isGrounded = false;
        jumpTimer = 0;   
        velocityY = -4;  
    }

    // 2. The Sustained Hold
    else if(!isGrounded && jumpButtonHeld && jumpTimer < MAX_JUMP_TIME){
        velocityY = -4; 
        jumpTimer++; 
    }

    // 3. THE FIX: The "Thruster Cut"
    // If you ever let go of the button while in the air...
    else if(!jumpButtonHeld) {
        // Instantly max out the timer! 
        // This ensures you cannot re-trigger step 2 if you press it again mid-air.
        jumpTimer = MAX_JUMP_TIME; 
    }

    // 4. Save the button state for the next frame
    wasJumpHeldLastFrame = jumpButtonHeld;
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
