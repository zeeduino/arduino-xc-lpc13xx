/*
  w_analog.cpp - Arduino-XC HAL API implementation of analog core library functions
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

#include "board_private.h"
#include "pins_arduino.h"
#include "board/pin_mapping.h"
#include "board/utilities.h"

uint32_t Board_Analog_Read(uint32_t adcChannel)
{
    uint16_t dataADC;
    uint32_t pinNumber;

    uint32_t port = 0;
    uint8_t pin = 7;
    uint32_t modefunc = WIRING_DIGITAL_NOTOK;

    if (!isAdcChannel(adcChannel))
    {
        return 0;
    }

    pinNumber = adcChannelToPin(adcChannel);
    port = APIN_PORT(pinNumber);
    pin = APIN_PIN(pinNumber);
    // by default, all AD pins are set to analog mode
    // so we can find mode value using APIN_MODE
    modefunc = APIN_MODE(pinNumber);
    Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, modefunc);

    /* Burst mode is disabled by default (see Chip_ADC_Init()) se we need to
     * enable A/D channel */
    Chip_ADC_EnableChannel(LPC_ADC, static_cast<ADC_CHANNEL_T>(adcChannel), ENABLE);

    /* Start A/D conversion */
    Chip_ADC_SetStartMode(LPC_ADC, ADC_START_NOW, ADC_TRIGGERMODE_RISING);

    /* Waiting for A/D conversion complete */
    while (Chip_ADC_ReadStatus(LPC_ADC, adcChannel, ADC_DR_DONE_STAT) != SET)
    {
    }

    /* Read ADC value */
    if (Chip_ADC_ReadValue(LPC_ADC, adcChannel, &dataADC) != SUCCESS)
        return 0;

    return dataADC;
}

// Right now, PWM output only works on the pins with
// hardware support.  These are defined in the appropriate
// pins_*.c file.  For the rest of the pins, we default
// to digital output.
// dutyCycle is set to 10-bit resolution (1024 steps)
void Board_Analog_Write(uint32_t ulPin, uint32_t ulValue)
{
    uint32_t dutyMatchCount;
    uint32_t pwmPeriod;

    LPC_TIMER_T *pTMR;
    uint8_t channel;
    uint8_t port;
    uint8_t pin;
    uint8_t pwmChannel;
    uint32_t modefunc;

    // analogWrite() calls this function only if ulPin has PWM support.
    // optimize this so we don't have to do it on every analogWrite
    port = APIN_PORT(ulPin);
    pin = APIN_PIN(ulPin);
    pwmChannel = APIN_PWM(ulPin);

    pTMR = PWM_MAP_TIMER(pwmChannel);
    channel = PWM_MAP_CHANNEL(pwmChannel);
    modefunc = PWM_MAP_MODE(pwmChannel);

    // Enable function on selected pin
    Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, modefunc);

    pwmPeriod = PWM_PERIOD;
    if(pTMR == LPC_TIMER16_0 || pTMR == LPC_TIMER16_1)
        pwmPeriod /= PWM_PRESCALE;

    // Hack to make output constantly LOW when dutyCycle == 0
    if(ulValue == 0)
    {
        dutyMatchCount = pwmPeriod + 1;
    }
    else
    {
        // If dutyCycle == max_value then output will constantly be HIGH
        dutyMatchCount = (pwmPeriod >> DUTY_CYCLE_RESOLUTION) * (((1 << DUTY_CYCLE_RESOLUTION) - 1) - ulValue);
    }

    Chip_TIMER_SetMatch(pTMR, channel, dutyMatchCount);
    Chip_TIMER_Enable(pTMR);
}
