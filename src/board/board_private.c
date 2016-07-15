/*
  board_private.h - platform specific internal initialization functions
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


/*****************************************
 * ADC
 *****************************************/

static ADC_CLOCK_SETUP_T ADCSetup;

void Board_ADC_Init(void)
{
    /*ADC Init */
    Chip_ADC_Init(LPC_ADC, &ADCSetup);
    Chip_ADC_EnableChannel(LPC_ADC, ADC_CH0, ENABLE);
}


/*****************************************
 * PWM
 *****************************************/

#define PWM_DUTY        50
#define PWMC_MAT0       0
#define PWMC_MAT1       1
#define PWMC_MAT2       2
#define PWMC_MAT3       3

void Board_PWM_Init_CT32B0()
{
    uint32_t dutyMatchCount;

    Chip_TIMER_Disable(LPC_TIMER32_0);

    Chip_TIMER_Init(LPC_TIMER32_0);

    // PIO1_25 -> MAT1, (IOCON_FUNC1 | IOCON_MODE_INACT)
    // PIO1_26 -> MAT2, (IOCON_FUNC1 | IOCON_MODE_INACT)
    // PIO1_27 -> MAT3, (IOCON_FUNC1 | IOCON_MODE_INACT)
//    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 25, (IOCON_FUNC1 | IOCON_MODE_INACT));
//    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 26, (IOCON_FUNC1 | IOCON_MODE_INACT));
//    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 27, (IOCON_FUNC1 | IOCON_MODE_INACT));

    // We set TIMER32_0 to reset TC on MR0 match
    // this effectively makes value written to MR0
    // to determine PWM signal frequency
    Chip_TIMER_ResetOnMatchEnable(LPC_TIMER32_0, 0);

    Chip_TIMER_SetMatch(LPC_TIMER32_0, 0, PWM_PERIOD);

    // This configures MR1 to act as PWM register
    // Value written to MR1 will determine when PIO1_25 goes high
    // which basically determines PWM duty cycle
    LPC_TIMER32_0->PWMC |= (1 << PWMC_MAT1);
    LPC_TIMER32_0->PWMC |= (1 << PWMC_MAT2);
    LPC_TIMER32_0->PWMC |= (1 << PWMC_MAT3);

    dutyMatchCount = PWM_PERIOD / 2; // 50% duty cycle
    Chip_TIMER_SetMatch(LPC_TIMER32_0, 1, dutyMatchCount);
    Chip_TIMER_SetMatch(LPC_TIMER32_0, 2, dutyMatchCount);
    Chip_TIMER_SetMatch(LPC_TIMER32_0, 3, dutyMatchCount);

//    Chip_TIMER_Enable(LPC_TIMER32_0);
}

void Board_PWM_Init_CT16B0()
{
    uint32_t periodRate;
    uint32_t dutyMatchCount;

    Chip_TIMER_Disable(LPC_TIMER16_0);

    Chip_TIMER_Init(LPC_TIMER16_0);

    // PIO1_13 -> MAT0, (IOCON_FUNC2 | IOCON_MODE_INACT)
    // PIO1_14 -> MAT1, (IOCON_FUNC2 | IOCON_MODE_INACT)
    // PIO1_15 -> MAT2, (IOCON_FUNC2 | IOCON_MODE_INACT)
//    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 13, (IOCON_FUNC2 | IOCON_MODE_INACT));
//    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 14, (IOCON_FUNC2 | IOCON_MODE_INACT));
//    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 15, (IOCON_FUNC2 | IOCON_MODE_INACT));

    // We set LPC_TIMER16_0 to reset TC on MR3 match
    // this effectively makes value written to MR3
    // to determine PWM signal frequency
    Chip_TIMER_ResetOnMatchEnable(LPC_TIMER16_0, 3);

    // if PWM_FREQ_HZ is lower than certain threshold,
    // periodRate will have value > 65535 (more than 16-bits)
    // in which case output frequency will not be the one
    // that was desired.
    // We have to prescale LPC_TIMER16_0 clock in order to
    // get frequencies < ~1100Hz when peripheral clock (PCLK)
    // is running on core frequency
    Chip_TIMER_PrescaleSet(LPC_TIMER16_0, PWM_PRESCALE - 1);
    periodRate = PWM_PERIOD / PWM_PRESCALE;
    Chip_TIMER_SetMatch(LPC_TIMER16_0, 3, periodRate);

    // This configures MR0 to act as PWM register
    // Value written to MR0 will determine when PIO1_13 goes high
    // which basically determines PWM duty cycle
    LPC_TIMER16_0->PWMC |= (1 << PWMC_MAT0);
    LPC_TIMER16_0->PWMC |= (1 << PWMC_MAT1);
    LPC_TIMER16_0->PWMC |= (1 << PWMC_MAT2);

    dutyMatchCount = periodRate / 2; // 50% duty cycle
    Chip_TIMER_SetMatch(LPC_TIMER16_0, 0, dutyMatchCount);
    Chip_TIMER_SetMatch(LPC_TIMER16_0, 1, dutyMatchCount);
    Chip_TIMER_SetMatch(LPC_TIMER16_0, 2, dutyMatchCount);

//    Chip_TIMER_Enable(LPC_TIMER16_0);
}

void Board_PWM_Init_CT16B1()
{
    uint32_t periodRate;
    uint32_t dutyMatchCount;

    Chip_TIMER_Disable(LPC_TIMER16_1);

    Chip_TIMER_Init(LPC_TIMER16_1);

    // PIO0_21 -> MAT0, (IOCON_FUNC1 | IOCON_MODE_INACT)
    // PIO1_13 -> MAT1, (IOCON_FUNC1 | IOCON_MODE_INACT)
//    Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 21, (IOCON_FUNC1 | IOCON_MODE_INACT));
//    Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 23, (IOCON_FUNC1 | IOCON_MODE_INACT));

    // We set LPC_TIMER16_1 to reset TC on MR3 match
    // this effectively makes value written to MR3
    // to determine PWM signal frequency
    Chip_TIMER_ResetOnMatchEnable(LPC_TIMER16_1, 3);

    // if PWM_FREQ_HZ is lower than certain threshold,
    // periodRate will have value > 65535 (more than 16-bits)
    // in which case output frequency will not be the one
    // that was desired.
    // We have to prescale LPC_TIMER16_1 clock in order to
    // get frequencies < ~1100Hz when peripheral clock (PCLK)
    // is running on core frequency
    Chip_TIMER_PrescaleSet(LPC_TIMER16_1, PWM_PRESCALE - 1);
    periodRate = PWM_PERIOD / PWM_PRESCALE;
    Chip_TIMER_SetMatch(LPC_TIMER16_1, 3, periodRate);

    // This configures MR0 to act as PWM register
    // Value written to MR0 will determine when PIO0_21 goes high
    // which basically determines PWM duty cycle
    LPC_TIMER16_1->PWMC |= (1 << PWMC_MAT0);
    LPC_TIMER16_1->PWMC |= (1 << PWMC_MAT1);

    dutyMatchCount = periodRate / 2; // 50% duty cycle
    Chip_TIMER_SetMatch(LPC_TIMER16_1, 0, dutyMatchCount);
    Chip_TIMER_SetMatch(LPC_TIMER16_1, 1, dutyMatchCount);

//    Chip_TIMER_Enable(LPC_TIMER16_1);
}
