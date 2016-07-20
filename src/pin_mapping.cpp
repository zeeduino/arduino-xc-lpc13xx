/*
  pin_mapping.cpp - implementation of pin mapping and handling utility functions
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

#include "board/pin_mapping.h"
#include "pins_arduino.h"

bool isPinPwm(uint32_t ulPin)
{
    return (APIN_PWM(ulPin) != NOT_ON_PWM);
}

uint32_t pinToPwmChannel(uint32_t ulPin)
{
    return NOT_A_PIN;
}

bool isPwmChannel(uint32_t ulPwmCahnnel)
{
    return (ulPwmCahnnel >= PWM_0) && (ulPwmCahnnel < PWM_CHANNEL_COUNT);
}

bool isInterruptNumber(uint32_t ulIntNumber)
{
    return (ulIntNumber < EXT_INT_COUNT);
}

uint32_t interruptNumberToPin(uint32_t intNo)
{
    uint32_t pinNumber;

    for(pinNumber = 0; pinNumber < arduinoPinsArrayLength; pinNumber++)
    {
        if((uint32_t)(APIN_INT(pinNumber)) == intNo)
        {
            return pinNumber;
        }
    }

    return NOT_A_PIN;
}

bool isAdcChannel(uint32_t ulAdcCahnnel)
{
    return (ulAdcCahnnel >= ADC_0) && (ulAdcCahnnel < ADC_CHANNEL_COUNT);
}

uint32_t adcChannelToPin(uint32_t adcChannel)
{
    uint32_t pinNumber;

    for(pinNumber = 0; pinNumber < arduinoPinsArrayLength; pinNumber++)
    {
        if((uint32_t)(APIN_ADC(pinNumber)) == adcChannel)
        {
            return pinNumber;
        }
    }

    return NOT_A_PIN;
}

uint32_t pinToAdcChannel(uint32_t ulPin)
{
    return (uint32_t) APIN_ADC(ulPin);
}

uint32_t pinCount(void)
{
    return arduinoPinsArrayLength;
}

bool isPin(uint32_t ulPin)
{
    if (ulPin >= arduinoPinsArrayLength) return false;

    if (APIN_PORT(ulPin) == 99) return false;

    return true;
}

