/*
  board)init.cpp - board initialization
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
#include "board/variant.h"

#include "chip.h"
#include "gpio_13xx_1.h"
#include "adc_13xx.h"

void Board_Init(void)
{
    /* Booting from FLASH, so remap vector table to FLASH */
    Chip_SYSCTL_Map(REMAP_USER_FLASH_MODE);

    /* Enable IOCON clock */
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);
    /* Set up and initialize clocking prior to call to main */
    Chip_SetupXtalClocking();
    /* Updates System Clock Frequency (Core Clock) library variable */
    SystemCoreClockUpdate();

    /* Initialize GPIO */
    Chip_GPIO_Init(LPC_GPIO_PORT);

    /* Sets up board pin muxing */
    Variant_Pins_Init();
}
