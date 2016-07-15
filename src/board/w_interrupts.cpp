/*
  w_interrupts.cpp - Arduino-XC HAL API implementation of interrupts related core library functions
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
#include "pinint_13xx.h"

#include "pins_arduino.h"
#include "wiring_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*isrCallback)(void);

static isrCallback callbackPinIntA = (isrCallback)NULL;
static isrCallback callbackPinIntB = (isrCallback)NULL;
static isrCallback callbackPinIntC = (isrCallback)NULL;
static isrCallback callbackPinIntD = (isrCallback)NULL;

/**
 * @brief   Handle interrupt from GPIO pin or GPIO pin mapped to PININT
 * @return  Nothing
 */
void PIN_INT0_IRQHandler(void)
{
    Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(0));
    if(callbackPinIntA) callbackPinIntA();
}

void PIN_INT1_IRQHandler(void)
{
    Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(1));
    if(callbackPinIntB) callbackPinIntB();
}

void PIN_INT2_IRQHandler(void)
{
    Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(2));
    if(callbackPinIntC) callbackPinIntC();
}

void PIN_INT3_IRQHandler(void)
{
    Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(3));
    if(callbackPinIntD) callbackPinIntD();
}
#ifdef __cplusplus
}
#endif


/*
 *     LOW to trigger the interrupt whenever the pin is low,
    CHANGE to trigger the interrupt whenever the pin changes value
    RISING to trigger when the pin goes from low to high,
    FALLING for when the pin goes from high to low.
    HIGH
 */

extern const uint32_t arduinoPinsArrayLength;

void Board_Attach_Interrupt(uint32_t ulPin, void (*callback)(void), uint32_t mode)
{
    uint8_t port = 1;
    uint8_t pin = 24;
    uint8_t pinIntChannel = 0;
    LPC1347_IRQn_Type pinIntIRQ = PIN_INT0_IRQn;

    port = APIN_PORT(ulPin);
    pin = APIN_PIN(ulPin);

    pinIntChannel = APIN_INT(ulPin);

    if(pinIntChannel == EXT_INT_0)
    {
        pinIntIRQ = PIN_INT0_IRQn;
        callbackPinIntA = callback;
    }
    else if(pinIntChannel == EXT_INT_1)
    {
        pinIntIRQ = PIN_INT1_IRQn;
        callbackPinIntB = callback;
    }
    else if(pinIntChannel == EXT_INT_2)
    {
        pinIntIRQ = PIN_INT2_IRQn;
        callbackPinIntC = callback;
    }
    else if(pinIntChannel == EXT_INT_3)
    {
        pinIntIRQ = PIN_INT3_IRQn;
        callbackPinIntD = callback;
    }

    /* Configure GPIO pin as input */
    Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT, port, pin);

    /* Configure pin as GPIO with pulldown */
    // All digital pins are selected such that GPIO is on IOCON_FUNC0
    Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, (IOCON_FUNC0 | IOCON_MODE_PULLDOWN));

    /* Enable PININT clock */
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_PINT);

    /* Configure interrupt channel for the GPIO pin in SysCon block */
    Chip_SYSCTL_SetPinInterrupt(pinIntChannel, port, pin);

    /* Configure channel interrupt as edge sensitive and falling edge interrupt */
    Chip_PININT_ClearIntStatus(LPC_PININT, PININTCH(pinIntChannel));

    if(mode == HIGH)
    {
        Chip_PININT_SetPinModeLevel(LPC_PININT, PININTCH(pinIntChannel));
        Chip_PININT_EnableIntHigh(LPC_PININT, PININTCH(pinIntChannel));
    }
    else if(mode == LOW)
    {
        Chip_PININT_SetPinModeLevel(LPC_PININT, PININTCH(pinIntChannel));
        Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(pinIntChannel));
    }
    else if(mode == RISING)
    {
        Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(pinIntChannel));
        Chip_PININT_EnableIntHigh(LPC_PININT, PININTCH(pinIntChannel));
    }
    else if(mode == FALLING)
    {
        Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(pinIntChannel));
        Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(pinIntChannel));
    }
    else if(mode == CHANGE)
    {
        Chip_PININT_SetPinModeEdge(LPC_PININT, PININTCH(pinIntChannel));
        Chip_PININT_EnableIntHigh(LPC_PININT, PININTCH(pinIntChannel));
        Chip_PININT_EnableIntLow(LPC_PININT, PININTCH(pinIntChannel));
    }

    /* Enable interrupt in the NVIC */
    NVIC_ClearPendingIRQ(pinIntIRQ);
    NVIC_EnableIRQ(pinIntIRQ);
}

void Board_Detach_Interrupt(uint32_t intNo)
{
    LPC1347_IRQn_Type pinIntIRQ = PIN_INT0_IRQn;

    if(intNo == EXT_INT_0)
    {
        pinIntIRQ = PIN_INT0_IRQn;
        callbackPinIntA = (isrCallback)NULL;
    }
    else if(intNo == EXT_INT_1)
    {
        pinIntIRQ = PIN_INT1_IRQn;
        callbackPinIntB = (isrCallback)NULL;
    }
    else if(intNo == EXT_INT_2)
    {
        pinIntIRQ = PIN_INT2_IRQn;
        callbackPinIntC = (isrCallback)NULL;
    }
    else if(intNo == EXT_INT_3)
    {
        pinIntIRQ = PIN_INT3_IRQn;
        callbackPinIntD = (isrCallback)NULL;
    }

    /* Disable interrupt in the NVIC */
    NVIC_ClearPendingIRQ(pinIntIRQ);
    NVIC_DisableIRQ(pinIntIRQ);
}



