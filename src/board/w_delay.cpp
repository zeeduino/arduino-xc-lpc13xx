/*
  w_delay.cpp - Arduino-XC HAL API implementation of delay/timer core library functions
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

#include "chip.h"
#include "timer_13xx.h"


static uint32_t ticksPerSecond = 0;
static uint32_t ticksPerMs = 0;
static uint32_t ticksPerUs = 0;

uint32_t Board_Delay_Millis( void )
{
    return Chip_TIMER_ReadCount(LPC_TIMER32_1) / ticksPerMs;
}

uint32_t Board_Delay_MillisMax( void )
{
    // for 32-bit timer, max counter value is 2^32-1 = 0xFFFFFFFF
    return 0xFFFFFFFF / ticksPerMs;
}

uint32_t Board_Delay_Micros( void )
{
    return Chip_TIMER_ReadCount(LPC_TIMER32_1) / ticksPerUs;
}

uint32_t Board_Delay_MicrosMax( void )
{
    // for 32-bit timer, max counter value is 2^32-1 = 0xFFFFFFFF
    return 0xFFFFFFFF / ticksPerUs;
}

//uint32_t Board_Delay_Start( void )
//{
//    return (Chip_TIMER_ReadCount(LPC_TIMER32_1) - ticksStart) / ticksPerUs;
//}

void Board_Delay_InitTimer( void )
{
    /* Use timer 1. Set prescaler to divide by 8, should give ticks at 9 MHz.
     * which is 9 ticks per uS. That is 9,000,000 ticks per second.
     * 32-bit unsigned int will overflow at 4294967296, which divided by
     * 9,000,000 gives ~477 seconds.
     * This means that timer will overflow after ~8 minutes.
     */
    const uint32_t prescaleDivisor = 8;
    Chip_TIMER_Init(LPC_TIMER32_1);
    Chip_TIMER_Reset(LPC_TIMER32_1);
    Chip_TIMER_PrescaleSet(LPC_TIMER32_1, prescaleDivisor - 1);
    Chip_TIMER_Enable(LPC_TIMER32_1);

    /* Pre-compute tick rate. */
    ticksPerSecond = Chip_Clock_GetSystemClockRate() / prescaleDivisor;
    ticksPerMs = ticksPerSecond / 1000;
    ticksPerUs = ticksPerSecond / 1000000;
}
