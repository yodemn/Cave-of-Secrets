// ECE319K_Lab9H/PCBJoystick.h
#ifndef PCBJOYSTICK_H
#define PCBJOYSTICK_H

#include <stdint.h>

void PCBJoystick_Init(void);
void PCBJoystick_In(uint32_t *x, uint32_t *y, uint32_t *select);

#endif
