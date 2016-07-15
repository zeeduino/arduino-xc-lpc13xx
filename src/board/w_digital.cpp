/*
  w_digital.cpp - Arduino-XC HAL API implementation of digital core library functions
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
#include "gpio_13xx_1.h"

#include "pins_arduino.h"
#include "wiring_constants.h"
#include "board/pin_mapping.h"

#include "board_private.h"

void Board_Digital_PinMode( uint32_t ulPin, uint32_t ulMode )
{
    uint32_t port = 0;
    uint8_t pin = 7;
    LPC_GPIO_T *pPORT;
    uint32_t modefunc = WIRING_DIGITAL_NOTOK;
    bool direction = false; // input

    // Check for ulPin range
    if(!isPin(ulPin))
        return;

    port = APIN_PORT(ulPin);
    pin = APIN_PIN(ulPin);
    pPORT = DIG_MAP_PORT(ulPin);
    modefunc = DIG_MAP_MODE(ulPin);

    if(ulMode == INPUT)
    {
        direction = false;
    }
    else if (ulMode == INPUT_PULLUP)
    {
        modefunc |= DIG_MAP_PUP(ulPin);
        direction = false;
    }
    else if (ulMode == OUTPUT)
    {
        direction = true;
    }

    if(modefunc != WIRING_DIGITAL_NOTOK)
    {
        Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, modefunc);
        Chip_GPIO_WriteDirBit(pPORT, port, pin, direction);
    }
}

void Board_Digital_Write( uint32_t ulPin, uint32_t ulVal )
{
    uint32_t port = 0;
    uint8_t pin = 7;
    LPC_GPIO_T *pPORT;

    bool val = ulVal == HIGH ? true : false;

    if(!isPin(ulPin))
        return;

    port = APIN_PORT(ulPin);
    pin = APIN_PIN(ulPin);
    pPORT = DIG_MAP_PORT(ulPin);

    Chip_GPIO_WritePortBit(pPORT, port, pin, val);
}


bool Board_Digital_Read( uint32_t ulPin )
{
    uint32_t port = 0;
    uint8_t pin = 7;
    LPC_GPIO_T *pPORT;

    if(!isPin(ulPin))
        return false;

    port = APIN_PORT(ulPin);
    pin = APIN_PIN(ulPin);
    pPORT = DIG_MAP_PORT(ulPin);

    return Chip_GPIO_GetPinState(pPORT, port, pin);
}

