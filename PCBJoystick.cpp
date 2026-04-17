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
  ADC1->ULLMEM.GPRCM.CLKCFG = 0xA9000000; // ULPCLK
  ADC1->ULLMEM.CLKFREQ = 7;               // 40-48 MHz
  ADC1->ULLMEM.CTL0 = 0x03010000;         // divide by 8
  ADC1->ULLMEM.CTL1 = 0x00010000;         // sequence mode
  ADC1->ULLMEM.CTL2 = 0x02010000;         // start MEMCTL[1], end MEMCTL[2]
  ADC1->ULLMEM.MEMCTL[1] = JOY_X_CHANNEL;
  ADC1->ULLMEM.MEMCTL[2] = JOY_Y_CHANNEL;
  ADC1->ULLMEM.SCOMP0 = 0;                // 8 sample clocks
  ADC1->ULLMEM.CPU_INT.IMASK = 0;         // no interrupt

  // PA28 input with pull-down, assuming button drives PA28 high when pressed
  IOMUX->SECCFG.PINCM[PA28INDEX] = 0x00050081;
}

void PCBJoystick_In(uint32_t *x, uint32_t *y, uint32_t *select){
  ADC1->ULLMEM.CTL0 |= 0x00000001;        // enable conversions
  ADC1->ULLMEM.CTL1 |= 0x00000100;        // start ADC sequence
  uint32_t volatile delay = ADC1->ULLMEM.STATUS;
  while((ADC1->ULLMEM.STATUS & 0x01) == 0x01){}
  *x = ADC1->ULLMEM.MEMRES[1];
  *y = ADC1->ULLMEM.MEMRES[2];
  *select = ((GPIOA->DIN31_0 & JOY_SEL_PIN) != 0);
}
