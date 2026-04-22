// Sound.cpp
// Runs on MSPM0
// Sound assets in sounds/sounds.h
// your name
// your data 
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "Sound.h"
#include "sounds/sounds.h"
#include "../inc/DAC5.h"
#include "../inc/Timer.h"


static const uint32_t SOUND_PERIOD = 80000000 / 11025;

static const uint8_t *SoundPt = 0;
static uint32_t SoundIndex = 0;
static uint32_t SoundCount = 0;
static uint8_t SoundActive = 0;

void SysTick_IntArm(uint32_t period, uint32_t priority){
  // write this
  SysTick->CTRL = 0;
  SysTick->LOAD = period - 1;
  SysTick->VAL = 0;

  SCB->SHP[1] = (SCB->SHP[1] & ~0xC0000000) | (priority << 30);
}
// initialize a 11kHz SysTick, however no sound should be started
// initialize any global variables
// Initialize the 5 bit DAC
void Sound_Init(void){
// write this
  DAC5_Init();

  SoundPt = 0;
  SoundIndex = 0;
  SoundCount = 0;
  SoundActive = 0;

  DAC5_Out(16); // middle value, quiet/resting
  SysTick_IntArm(SOUND_PERIOD, 1);
}
extern "C" void SysTick_Handler(void);
void SysTick_Handler(void){ // called at 11 kHz
  // output one value to DAC if a sound is active
    // output one value to DAC if a sound is active
  if(SoundActive && (SoundIndex < SoundCount)){
    DAC5_Out(SoundPt[SoundIndex] >> 3);
    SoundIndex++;
  } else {
    SoundActive = 0;
    SoundPt = 0;
    SoundIndex = 0;
    SoundCount = 0;
    DAC5_Out(16);
    SysTick->CTRL = 0;
  }
}

//******* Sound_Start ************
// This function does not output to the DAC. 
// Rather, it sets a pointer and counter, and then enables the SysTick interrupt.
// It starts the sound, and the SysTick ISR does the output
// feel free to change the parameters
// Sound should play once and stop
// Input: pt is a pointer to an array of DAC outputs
//        count is the length of the array
// Output: none
// special cases: as you wish to implement
void Sound_Start(const uint8_t *pt, uint32_t count){
// write this
  if((pt == 0) || (count == 0)){
    return;
  }
  __disable_irq();

  // update stuff
  SoundPt = pt;
  SoundCount = count;
  SoundIndex = 0;
  SoundActive = 1;
  SysTick->VAL = 0;
  
  SysTick->CTRL = 0x00000007;
  __enable_irq();
}

void Sound_Shoot(void){
// write this
  Sound_Start(shoot, 4080);
}

void Sound_Chest(void) {
  Sound_Start(chest, chestLength);
}

void Sound_Death(void) {
  Sound_Start(death, deathLength);
}

void Sound_Killed(void){
// write this

}
void Sound_Explosion(void){
// write this

}

void Sound_Fastinvader1(void){

}
void Sound_Fastinvader2(void){

}
void Sound_Fastinvader3(void){

}
void Sound_Fastinvader4(void){

}
void Sound_Highpitch(void){

}
