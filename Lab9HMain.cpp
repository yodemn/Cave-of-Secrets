// Lab9HMain.cpp
// Runs on MSPM0G3507
// Lab 9 ECE319H
// Your name
// Last Modified: January 12, 2026

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
#include "images/images.h"
#include "Sprites.h"
#include "Backgrounds.h"
#include "TileSprites.h"
#include "Level.h"
#include "PCBJoystick.h"

extern "C" void __disable_irq(void);
extern "C" void __enable_irq(void);
extern "C" void TIMG12_IRQHandler(void);
// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}
 

SlidePot Sensor(1674,173); // copy calibration from Lab 7
static const int16_t PlayerStartX = 24;
static const int16_t PlayerStartY = 118 - TILE_SPRITE_HEIGHT;
AnimatedPlayer player(PlayerStartX, PlayerStartY);
Background back1(0, 127, 0, background0_img);
static const uint8_t CurrentLevelIndex = 0;
static uint32_t GetJoystickDirection(uint32_t x, uint32_t y, uint32_t select);
static Rect GetPlayerArea(int16_t x, int16_t y);

volatile uint32_t CurrentJoystickX = 2048;
volatile uint32_t CurrentJoystickY = 2048;
volatile uint32_t CurrentJoystickSelect = 0;
volatile uint32_t CurrentJoystickDirection = 0;
volatile uint8_t CurrentJumpButtonHeld = 0;
volatile uint8_t NewFrameReady = 0;
bool chestLEDOn = false;

// game engine runs at 30Hz
void TIMG12_IRQHandler(void){
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    uint32_t x, y, select;
    PCBJoystick_In(&x, &y, &select);
    CurrentJoystickX = x;
    CurrentJoystickY = y;
    CurrentJoystickSelect = select;
    CurrentJoystickDirection = GetJoystickDirection(x, y, select);

    uint32_t portA_input = GPIOA->DIN31_0;
    CurrentJumpButtonHeld = ((portA_input & (1<<27)) != 0);
    NewFrameReady = 1;
  }
}
uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
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
// use main1 to observe special characters
int main1(void){ // main1
    char l;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_FillScreen(0x0000);            // set screen to black
  for(int myPhrase=0; myPhrase<= 2; myPhrase++){
    for(int myL=0; myL<= 3; myL++){
         ST7735_OutString((char *)Phrases[LANGUAGE][myL]);
      ST7735_OutChar(' ');
         ST7735_OutString((char *)Phrases[myPhrase][myL]);
      ST7735_OutChar(13);
    }
  }
  Clock_Delay1ms(3000);
  ST7735_FillScreen(0x0000);       // set screen to black
  l = 128;
  while(1){
    Clock_Delay1ms(2000);
    for(int j=0; j < 3; j++){
      for(int i=0;i<16;i++){
        ST7735_SetCursor(7*j+0,i);
        ST7735_OutUDec(l);
        ST7735_OutChar(' ');
        ST7735_OutChar(' ');
        ST7735_SetCursor(7*j+4,i);
        ST7735_OutChar(l);
        l++;
      }
    }
  }
}

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

static uint32_t GetJoystickDirection(uint32_t x, uint32_t y, uint32_t select){
  if(select){
    return 5; // button
  }
  // if(x < 1500){
  //   return 4; // down
  // }
  // if(x > 2600){
  //   return 3; // up
  // }
  if(y < 2000){
    return 1; // left
  }
  if(y > 2100){
    return 2; // right
  }
  return 0; // center
}

// active game main
int main(void) {
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  PCBJoystick_Init();
  Switch_Init();   // Initialize your switches
  LED_Init();      // Initialize your LEDs

  ST7735_InitPrintf(INITR_BLACKTAB); 
  ST7735_SetRotation(1);
  ST7735_FillScreen(ST7735_BLACK);

  ResetLevelObjectStates(CurrentLevelIndex);
  back1.Draw();
  DrawLevel(CurrentLevelIndex);
  player.Draw();

  TimerG12_IntArm(80000000 / 30, 2); // 30 Hz game/input tick at 80 MHz
  __enable_irq();

  bool wasInteractButtonHeld = false;
  while(1){
    if(NewFrameReady == 0){
      continue;
    }
    
    __disable_irq();
    uint32_t direction = CurrentJoystickDirection;
    bool jumpButtonHeld = (CurrentJumpButtonHeld != 0);
    bool interactButtonHeld;
    if((GPIOA->DIN31_0 & (1<<26)) != 0) {
      interactButtonHeld = true;
    }  else {
      interactButtonHeld = false;
    }
    NewFrameReady = 0;
    __enable_irq();

    Rect previousPlayerArea = GetPlayerArea(player.x, player.y);

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

    if(player.isGrounded && player.y < 127 && !IsPlayerSupportedByPlatform(CurrentLevelIndex, GetPlayerArea(player.x, player.y))){
      player.StartFalling();
    }

    player.Jump(jumpButtonHeld);
    player.UpdatePhysics();

    int16_t platformLandingY;
    Rect currentPlayerArea = GetPlayerArea(player.x, player.y);
    if(FindPlatformLanding(CurrentLevelIndex, previousPlayerArea, currentPlayerArea, &platformLandingY)){
      player.LandOn(platformLandingY);
      currentPlayerArea = GetPlayerArea(player.x, player.y);
    }

    player.Update();

    Rect redrawArea = CombineAreas(previousPlayerArea, currentPlayerArea);

    bool interactButtonPressed = interactButtonHeld && !wasInteractButtonHeld;
    Rect chestArea;
    uint32_t count;
    if(interactButtonPressed && TryOpenNearbyChest(CurrentLevelIndex, currentPlayerArea, &chestArea)){
      redrawArea = CombineAreas(redrawArea, chestArea);
      chestLEDOn = true;
      LED_On(1<<16);
      count = 15;
    }

    if(chestLEDOn && count <= 0) {
      count = 0;
      LED_Off(1<<16);
      chestLEDOn = false;
    } else if (chestLEDOn) {
      count--;
    }
    wasInteractButtonHeld = interactButtonHeld;

    Rect animatedChestArea;
    if(UpdateLevelAnimations(CurrentLevelIndex, &animatedChestArea)){
      redrawArea = CombineAreas(redrawArea, animatedChestArea);
    }
    
    RedrawChangedArea(redrawArea);
    player.Draw();
  }
}

// use main2 to observe graphics
int main2(void){ // main2
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  PCBJoystick_Init();
  Switch_Init();   // Initialize your switches (PB24 - PB27)
  LED_Init();      // Initialize your LEDs (PB15 - PB17)
  ST7735_InitPrintf(INITR_BLACKTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_SetRotation(1);
  ST7735_FillScreen(ST7735_BLACK);
  // ST7735_DrawBitmap(22, 159, PlayerShip0, 18,8); // player ship bottom
  // ST7735_DrawBitmap(53, 151, Bunker0, 18,5);
  // ST7735_DrawBitmap(42, 159, PlayerShip1, 18,8); // player ship bottom
  // ST7735_DrawBitmap(62, 159, PlayerShip2, 18,8); // player ship bottom
  // ST7735_DrawBitmap(82, 159, PlayerShip3, 18,8); // player ship bottom
  // ST7735_DrawBitmap(0, 9, SmallEnemy10pointA, 16,10);
  // ST7735_DrawBitmap(20,9, SmallEnemy10pointB, 16,10);
  // ST7735_DrawBitmap(40, 9, SmallEnemy20pointA, 16,10);
  // ST7735_DrawBitmap(60, 9, SmallEnemy20pointB, 16,10);
  // ST7735_DrawBitmap(80, 9, SmallEnemy30pointA, 16,10);
  //playerSprite.Draw();
  LED_On(1<<15);
  LED_On(1<<16);
  LED_On(1<<17);
  back1.Draw();
  DrawLevel(CurrentLevelIndex);
  // ST7735_FillScreen(0x0000);   // set screen to black
  // ST7735_SetCursor(1, 1);
  // ST7735_OutString((char *)"GAME OVER");
  // ST7735_SetCursor(1, 2);
  // ST7735_OutString((char *)"Nice try,");
  // ST7735_SetCursor(1, 3);
  // ST7735_OutString((char *)"Earthling!");
  // ST7735_SetCursor(2, 4);
  // ST7735_OutUDec(1234);
  while(1){
  }
}
int mainA(void){ 
  __disable_irq();
  PLL_Init();      // Set bus speed
  LaunchPad_Init(); // Basic MSPM0 setup
  Switch_Init();   // Initialize your switches (PB24 - PB27)
  LED_Init();      // Initialize your LEDs (PB15 - PB17)
  __enable_irq();

  // Test: Turn on the Red LED (PB15)
  LED_On(1<<15);
  LED_On(1<<16);
  LED_On(1<<17);

  while(1){
      // The infinite loop keeps the program running.
      // The LED will stay on!
  }
}
// use main3 to test switches and LEDs
int main3(void){ // main3
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  while(1){
      // Switch_In returns PA24-PA27 as bits 0-3.
      uint32_t input = Switch_In();
      
      // Test Switch 1 (PA24) -> Controls LED 1 (PA15)
      if(input & 0x01){
          LED_On(1<<15);
      } else {
          LED_Off(1<<15);
      }
      
      // Test Switch 2 (PA25) -> Controls LED 2 (PA16)
      if(input & 0x02){
          LED_On(1<<16);
      } else {
          LED_Off(1<<16);
      }
      
      // Test Switch 3 (PA26) -> Controls LED 3 (PA17)
      if(input & 0x04){
          LED_On(1<<17);
      } else {
          LED_Off(1<<17);
      }
      
      // Test Switch 4 (PA27) -> Toggles all LEDs while held
      if(input & 0x08){
          LED_Toggle((1<<15) | (1<<16) | (1<<17));
          // Small delay so it doesn't toggle millions of times per second
          for(volatile int i=0; i<100000; i++){}; 
      }
  }
}
// use main4 to test sound outputs
int main4(void){ uint32_t last=0,now;
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  Switch_Init(); // initialize switches
  LED_Init(); // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(ADC0,6,0); // ADC1 channel 6 is PB20, TExaS scope
  __enable_irq();
  while(1){
    now = Switch_In(); // one of your buttons
    if((last == 0)&&(now == 1)){
      Sound_Shoot(); // call one of your sounds
    }
    if((last == 0)&&(now == 2)){
      Sound_Killed(); // call one of your sounds
    }
    if((last == 0)&&(now == 4)){
      Sound_Explosion(); // call one of your sounds
    }
    if((last == 0)&&(now == 8)){
      Sound_Fastinvader1(); // call one of your sounds
    }
    // modify this to test all your sounds
  }
}
// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main5(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf(INITR_REDTAB); // INITR_REDTAB for AdaFruit, INITR_BLACKTAB for HiLetGo
  ST7735_FillScreen(ST7735_BLACK);
  Sensor.Init(); // PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound
  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  
  // initialize all data structures
  __enable_irq();

  while(1){
    // wait for semaphore
       // clear semaphore
       // update ST7735R
    // check for end game or level switch
  }
}
