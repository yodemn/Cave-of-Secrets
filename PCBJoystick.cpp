#include <ti/devices/msp/msp.h>
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "PCBJoystick.h"

#define JOY_SEL_PIN (1U << 28)
#define JOY_X_CHANNEL 4U // PB17 = ADC1 channel 4
#define JOY_Y_CHANNEL 6U // PB19 = ADC1 channel 6

void PCBJoystick_Init(void){
  ADC1->ULLMEM.GPRCM.RSTCTL = 0xB1000003;
  ADC1->ULLMEM.GPRCM.PWREN = 0x26000001;
  Clock_Delay(24);
  ADC1->ULLMEM.GPRCM.CLKCFG = 0xA9000000;
  ADC1->ULLMEM.CLKFREQ = 7;
  ADC1->ULLMEM.CTL0 = 0x03010000;
  ADC1->ULLMEM.CTL1 = 0x00010000;
  ADC1->ULLMEM.CTL2 = 0x02010000;         
  ADC1->ULLMEM.MEMCTL[1] = JOY_X_CHANNEL;
  ADC1->ULLMEM.MEMCTL[2] = JOY_Y_CHANNEL;
  ADC1->ULLMEM.SCOMP0 = 0;               
  ADC1->ULLMEM.CPU_INT.IMASK = 0;        

  IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00050081;
}

void PCBJoystick_In(uint32_t *x, uint32_t *y, uint32_t *select){
  ADC1->ULLMEM.CTL0 |= 0x00000001;
  ADC1->ULLMEM.CTL1 |= 0x00000100;
  uint32_t volatile delay = ADC1->ULLMEM.STATUS;
  while((ADC1->ULLMEM.STATUS & 0x01) == 0x01){}
  *x = ADC1->ULLMEM.MEMRES[1];
  *y = ADC1->ULLMEM.MEMRES[2];
  *select = ((GPIOA->DIN31_0 & JOY_SEL_PIN) != 0);
}
