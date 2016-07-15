/*
  variant.cpp - implementation of initVariant() and other platform-specific Arduino functions
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

#include "board/board_variant.h"
#include "board_private.h"
#include "board/board_serial.h"

#include "chip.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * All LPC IRQ handlers and such have to be implemented here
 * within extern "C" declaration, otherwise their names will get mangled
 * by C++ compiler and they WON'T get linked where you would expect them
 * to be because they are declared with __attribute__((weak)).
 */

/**
 * @brief   Handle interrupt from SysTick timer
 * @return  Nothing
 */
void SysTick_Handler(void)
{
    //Board_LED_Toggle(0);
}

void yield(void)
{
    __WFI();
}

void initVariant( void )
{
	Board_Init();
    // Set systick timer to 1ms interval
    SysTick_Config(SystemCoreClock / 1000);

    Board_Delay_InitTimer();
    Board_ADC_Init();
    Board_PWM_Init_CT32B0();
    Board_PWM_Init_CT16B0();
    Board_PWM_Init_CT16B1();
    Serial_Init();
}


#ifdef __cplusplus
}
#endif
