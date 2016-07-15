/*
  board_private.h - functions specific to Arduino-XC HAL implementation
  Copyright (c) 2016 Ravendyne Inc.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef BOARD_PRIVATE_H_
#define BOARD_PRIVATE_H_

#include "board/board_serial.h"

// resolution in bits, 10-bit resolution ~ 1024 steps
#define DUTY_CYCLE_RESOLUTION   (10)
#define PWM_FREQ_HZ             (1000)
#define PWM_PRESCALE            (2)
#define PWM_PERIOD              (Chip_Clock_GetSystemClockRate() / PWM_FREQ_HZ)
#define WIRING_DIGITAL_NOTOK (0xFFFFFFFF)

#ifdef __cplusplus
extern "C" {
#endif

void Board_PWM_Init_CT32B0(void);
void Board_PWM_Init_CT16B0(void);
void Board_PWM_Init_CT16B1(void);
void Board_PWM_Set(uint8_t pwmChannel, uint8_t dutyCycle);

void Board_ADC_Init(void);
void Variant_Pins_Init(void);

#ifdef __cplusplus
}
#endif


#endif /* BOARD_PRIVATE_H_ */
