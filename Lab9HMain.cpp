// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/SlidePot.h"
#include "../inc/DAC5.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "SoundClash.h"
#include "images/images.h"
#include "Sprites.h"
#include "Backgrounds.h"
#include "TileSprites.h"
#include "Level.h"
#include "PCBJoystick.h"

extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);

void PLL_Init(void){ 
  Clock_Init80MHz(0);   
}

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}
 
SlidePot Sensor(1674,173); 

static const int16_t PlayerStart[] = {24, 118-TILE_SPRITE_HEIGHT, 24, 118-TILE_SPRITE_HEIGHT, 10, 50};
AnimatedPlayer player(PlayerStart[0], PlayerStart[1]);
Background back1(0, 127, 0, background0_img);
static uint8_t CurrentLevelIndex = 0;
static const uint32_t ChestGoalCount = 6;
static uint32_t GetJoystickDirection(uint32_t x, uint32_t y, uint32_t select);
static Rect GetPlayerArea(int16_t x, int16_t y);

volatile uint32_t CurrentJoystickX = 2048;
volatile uint32_t CurrentJoystickY = 2048;
volatile uint32_t CurrentJumpSelect = 0;
volatile uint32_t CurrentJoystickDirection = 0;
volatile uint8_t CurrentJumpButtonHeld = 0;
volatile uint8_t NewFrameReady = 0;
volatile uint32_t MenuButton = 0;
bool chestLEDOn = false;

static void FlashLevelCompleteLED(void){
  for(uint8_t i = 0; i < 4; i++){
    LED_On(1 << 17);
    Clock_Delay1ms(100);
    LED_Off(1 << 17);
    Clock_Delay1ms(100);
  }
}

// game engine runs at 30Hz
void TIMG12_IRQHandler(void){
  if((TIMG12->CPU_INT.IIDX) == 1){ 
    uint32_t x, y, select;
    PCBJoystick_In(&x, &y, &select);
    CurrentJoystickX = x;
    CurrentJoystickY = y;
    CurrentJumpSelect = select;
    CurrentJoystickDirection = GetJoystickDirection(x, y, select);

    uint32_t portA_input = GPIOA->DIN31_0;
    CurrentJumpButtonHeld = ((portA_input & (1<<27)) != 0);
    CurrentJumpSelect = ((portA_input & (1<<24)) != 0);
    MenuButton = ((portA_input & (1<<25)) != 0);
    NewFrameReady = 1;
  }
}

uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
int mylanguagenum = 0;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};

static Rect GetPlayerArea(int16_t x, int16_t y){
  Rect r;
  r.x0 = x;
  r.y0 = y - PLAYER_SPRITE_HEIGHT + 1;
  r.x1 = x + PLAYER_SPRITE_WIDTH - 1;
  r.y1 = y;
  return r;
}

static Rect CombineAreas(Rect a, Rect b){
  Rect r;
  r.x0 = (a.x0 < b.x0) ? a.x0 : b.x0;
  r.y0 = (a.y0 < b.y0) ? a.y0 : b.y0;
  r.x1 = (a.x1 > b.x1) ? a.x1 : b.x1;
  r.y1 = (a.y1 > b.y1) ? a.y1 : b.y1;
  return r;
}

static Rect PadArea(Rect area, int16_t padding){
  area.x0 -= padding;
  area.y0 -= padding;
  area.x1 += padding;
  area.y1 += padding;
  return area;
}

static void RestoreBackgroundArea(Rect outdatedArea){
  drawRowsOfColorForBackground(outdatedArea.x0, outdatedArea.y0, outdatedArea.x1, outdatedArea.y1);
}

static void RedrawChangedArea(Rect area){
  area = PadArea(area, 2);
  RestoreBackgroundArea(area);
  RedrawLevelPiecesInArea(CurrentLevelIndex, area);
}

static void DrawStarCounter(uint32_t stars){
  Rect hudArea = {0, 0, 24, 15};
  if(stars > 9){
    stars = 9;
  }
  RestoreBackgroundArea(hudArea);
  DrawImageChroma(1, 13, StarCounterImage);
  ST7735_DrawCharS(13, 4, '0' + stars, ST7735_YELLOW, ST7735_YELLOW, 1);
}

static uint32_t GetJoystickDirection(uint32_t x, uint32_t y, uint32_t select){
  if(select){
    return 5; // button
  }
  if(y < 2000){
    return 1; // left
  }
  if(y > 2100){
    return 2; // right
  }
  return 0; // center
}

int RunMenuScreen(const char* title, const char* options[], int numOptions) {
    int selectedItem = 0; 
    bool joystickReset = true; 
    int flashcur = 30;

    mainMenu(title, options, numOptions, mylanguagenum);

    // CRITICAL: Wait for the player to let go of the button first!
    while(MenuButton != 0) { } 

    // The Universal Loop
    while(MenuButton == 0){
        if(NewFrameReady){
            NewFrameReady = 0;
            uint32_t rawX = CurrentJoystickX; 
            int menuDir = 0;
            
            if(rawX < 1500) { menuDir = 4; } // Down
            if(rawX > 2600) { menuDir = 3; } // Up

            if(menuDir == 0) { joystickReset = true; }

            if(joystickReset) {
                if(menuDir == 4 && selectedItem < (numOptions - 1)) { // Down
                    ST7735_SetCursor(5, 5 + (selectedItem * 2)); 
                    ST7735_OutChar(' '); 
                    selectedItem++;
                    flashcur = 30; joystickReset = false; 
                } 
                else if(menuDir == 3 && selectedItem > 0) { // Up
                    ST7735_SetCursor(5, 5 + (selectedItem * 2)); 
                    ST7735_OutChar(' '); 
                    selectedItem--;
                    flashcur = 30; joystickReset = false; 
                }
            }

            // Blinking Cursor
            flashcur--;
            if (flashcur <= 0) flashcur = 30;

            if (flashcur > 15) {
                ST7735_SetCursor(5, 5 + (selectedItem * 2)); ST7735_OutChar('>'); 
            } else {
                ST7735_SetCursor(5, 5 + (selectedItem * 2)); ST7735_OutChar(' '); 
            }      
        }
    }
    
    // Wait for them to let go of the button before returning
    while(MenuButton != 0) { } 

    return selectedItem; // Returns 0, 1, 2, etc. depending on what they picked!
}

// active game main
int main(void) {
  __disable_irq();
  PLL_Init(); 
  LaunchPad_Init();
  PCBJoystick_Init();
  Switch_Init();   
  LED_Init();      
  Sound_Init();

  ST7735_InitPrintf(INITR_BLACKTAB); 
  ST7735_SetRotation(1);
  ST7735_FillScreen(ST7735_BLACK);

  TimerG12_IntArm(80000000 / 30, 2); // 30 Hz game/input tick
  __enable_irq();
  Sound_Start(ClashSound, sizeof(ClashSound));
  
  // --- DEFINE YOUR MENUS ---
  const char* mainOptions[] = {"Start Game", "Tutorial", "Languages", "Iniciar juego", "Tutorial","Idiomas"};
  const char* tutorialOptions[] = {"Back to Menu","Down = Jump", "Left = ENTER", "Right = Interact","Volver al menu","Abajo = Saltar", "Izq = Entrar", "Der = Accion"};
  const char* langOptions[] = {"English", "Espanol", "Back","English", "Espanol", "Volver"};
  const char* languagePage[] = {"Select Lang", "Seleccionar idioma"};
  const char* tutorialPage[] = {"Tutorial", "Tutorial"};
  const char* TitlePage[] = {"Cave of Secrets", "Cueva de los secretos"};
  
  // --- MASTER GAME LOOP ---
  while(1){
    int menuState = 0;
    bool readyToPlay = false;
  
    // --- THE MENU SYSTEM ---
    while(!readyToPlay) {
        if (menuState == 0) {
            int choice = RunMenuScreen(TitlePage[mylanguagenum], mainOptions, 3);
            if (choice == 0) readyToPlay = true; 
            if (choice == 1) menuState = 1;       
            if (choice == 2) menuState = 2;       
        }
        else if (menuState == 1) {
            int choice = RunMenuScreen(tutorialPage[mylanguagenum], tutorialOptions, 4);
            if (choice == 0) menuState = 0;       
        }
        else if (menuState == 2) {
            int choice = RunMenuScreen(languagePage[mylanguagenum], langOptions, 3);
            if (choice == 0) {
              myLanguage = English;
              mylanguagenum = 0;
            }
            if (choice == 1) {
              myLanguage = Spanish;
              mylanguagenum = 1;
            }
            menuState = 0; 
        }
    }

    // --- MENU FINISHED - START GAME SETUP ---
    ST7735_FillScreen(ST7735_BLACK);
    
    // Set the player's spawn based on CurrentLevelIndex
    player.x = PlayerStart[CurrentLevelIndex*2];
    player.y = PlayerStart[(CurrentLevelIndex*2)+1];

    ResetLevelObjectStates(CurrentLevelIndex);
    back1.Draw();
    DrawLevel(CurrentLevelIndex);
    player.Draw();
    DrawStarCounter(0);

    bool wasInteractButtonHeld = false;
    uint32_t chestCount = ChestGoalCount;
    uint32_t starCount = 0;
    bool starCounterDirty = false;
    uint32_t chestLedTicks = 0;

    // --- ACTIVE GAME LOOP ---
    while(1){
      if(NewFrameReady == 0){
        continue;
      }
      
      __disable_irq();
      uint32_t direction = CurrentJoystickDirection;
      
      // Grab buttons safely from the IRQ variables
      bool jumpButtonHeld = (CurrentJumpButtonHeld != 0);
      bool joySelectHeld = (CurrentJumpSelect != 0); 
      
      bool interactButtonHeld;
      if((GPIOA->DIN31_0 & (1<<26)) != 0) {
        interactButtonHeld = true;
      }  else {
        interactButtonHeld = false;
      }
      NewFrameReady = 0;
      __enable_irq();

      // --- MENU RETURN LOGIC ---
      if (joySelectHeld) {
          // Wait for the player to physically let go of the joystick button!
          while(CurrentJumpSelect != 0) {} 
          // Break out of the active game loop to go back to the menu!
          break; 
      }

      Rect previousPlayerArea = GetPlayerArea(player.x, player.y);
      if(player.comboState != 2){
        if(direction == 1){
          player.SetFacingLeft(true);
          player.Move(PLAYER_RUN_SPEED);
          player.SetWalking(true);
        } else if(direction == 2){
          player.SetFacingLeft(false);
          player.Move(-PLAYER_RUN_SPEED);
          player.SetWalking(true);
        } else {
          player.SetWalking(false);
        }
      }
      else{
        player.SetWalking(false);
      }

      Rect movedPlayerArea = GetPlayerArea(player.x, player.y);
      int16_t blockedX;
      if(FindPlatformColumnSideCollision(CurrentLevelIndex, previousPlayerArea, movedPlayerArea, &blockedX)){
        player.x = blockedX;
      }

      if(player.isGrounded && player.y < 127 && !IsPlayerSupportedByPlatform(CurrentLevelIndex, GetPlayerArea(player.x, player.y))){
        player.StartFalling();
      }

      // Trigger jump
      player.Jump(jumpButtonHeld, joySelectHeld);
      player.UpdatePhysics();
      
      int16_t platformLandingY;
      Rect currentPlayerArea = GetPlayerArea(player.x, player.y);
      if(FindPlatformLanding(CurrentLevelIndex, previousPlayerArea, currentPlayerArea, &platformLandingY)){
        player.LandOn(platformLandingY);
        currentPlayerArea = GetPlayerArea(player.x, player.y);
      }

      if(IsPlayerTouchingSpike(CurrentLevelIndex, currentPlayerArea)) {
          LED_Off((7<<15));
          player.x = PlayerStart[CurrentLevelIndex*2];
          player.y = PlayerStart[(CurrentLevelIndex*2)+1];
          player.velocityY = 0;
          player.comboState = 0;
          
          chestCount = ChestGoalCount;
          starCount = 0;
          starCounterDirty = false;

          ResetLevelObjectStates(CurrentLevelIndex);

          LED_On((1<<15));
          Sound_Death();
          ST7735_FillScreen(ST7735_BLACK);
          Clock_Delay1ms(1000);
          LED_Off((1<<15));
          back1.Draw();
          DrawLevel(CurrentLevelIndex);
          player.Draw();
          DrawStarCounter(starCount);
          continue; 
      }
      player.Update();

      Rect redrawArea = CombineAreas(previousPlayerArea, currentPlayerArea);

      bool interactButtonPressed = interactButtonHeld && !wasInteractButtonHeld;
      Rect chestArea;
      if(interactButtonPressed && TryOpenNearbyChest(CurrentLevelIndex, currentPlayerArea, &chestArea)){
        Sound_Chest();
        redrawArea = CombineAreas(redrawArea, chestArea);
        chestLEDOn = true;
        LED_On(1<<16);
        chestCount--;
        starCount++;
        starCounterDirty = true;
        chestLedTicks = 15;
      }

      if(chestLEDOn && chestLedTicks <= 0) {
        chestLedTicks = 0;
        LED_Off(1<<16);
        chestLEDOn = false;
      } else if (chestLEDOn) {
        chestLedTicks--;
      }
      wasInteractButtonHeld = interactButtonHeld;
      
      Rect animatedChestArea;
      if(UpdateLevelAnimations(CurrentLevelIndex, &animatedChestArea)){
        redrawArea = CombineAreas(redrawArea, animatedChestArea);
      }
      
      RedrawChangedArea(redrawArea);
      
      // HUD Z-Order Fix
      if(redrawArea.x0 <= 30 && redrawArea.y0 <= 20) {
          starCounterDirty = true;
      }
      if(starCounterDirty){
        DrawStarCounter(starCount);
        starCounterDirty = false;
      }

      player.Draw();

      // Check Level Completion
      if(chestCount <= 0){
        LED_Off(1<<16);
        chestLEDOn = false;
        chestLedTicks = 0;
        FlashLevelCompleteLED();
        
        if(CurrentLevelIndex + 1 < LevelCount){
          CurrentLevelIndex++;
          player.x = PlayerStart[CurrentLevelIndex*2];
          player.y = PlayerStart[(CurrentLevelIndex*2)+1];
          player.velocityY = 0;
          player.comboState = 0;
          player.SetWalking(false);

          chestCount = ChestGoalCount;
          starCount = 0;
          starCounterDirty = false;

          ST7735_FillScreen(ST7735_BLACK);
          ResetLevelObjectStates(CurrentLevelIndex);
          back1.Draw();
          DrawLevel(CurrentLevelIndex);
          player.Draw();
          DrawStarCounter(starCount);
          continue;
        }
        
        // Game Won Screen!
        ST7735_FillScreen(ST7735_WHITE);
        while(1){
          // Stay here forever or add "You Win!" text
        }
      }
    }
  }
}