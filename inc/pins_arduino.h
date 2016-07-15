/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis
  Copyright (c) 2016 Ravendyne Inc. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include "chip.h"
#include "gpio_13xx_1.h"
#include "timer_13xx.h"

typedef enum _ExtInterruptType
{
  NO_EXT_INT=-1,
  EXT_INT_0=0,
  EXT_INT_1,
  EXT_INT_2,
  EXT_INT_3,
  EXT_INT_4,
  EXT_INT_5,
  EXT_INT_6,
  EXT_INT_7,
  EXT_INT_COUNT
} ExtInterruptType ;

typedef enum _AnalogChannelType
{
  NO_ADC=-1,
  ADC_0=0,
  ADC_1,
  ADC_2,
  ADC_3,
  ADC_4,
  ADC_5,
  ADC_6,
  ADC_7,
  ADC_CHANNEL_COUNT
} AnalogChannelType ;

typedef enum _PwmChannelType
{
  NOT_ON_PWM=-1,
  PWM_0=0,
  PWM_1,
  PWM_2,
  PWM_3,
  PWM_4,
  PWM_5,
  PWM_6,
  PWM_7,
  PWM_CHANNEL_COUNT
} PwmChannelType ;

typedef struct _PwmMappingType
{
    LPC_TIMER_T *pTMR;
    uint32_t channel:8;
    uint32_t reserved:8;
    uint32_t modefunc:16; // IOCON register value to connect PWM function to it's pin designated in g_ArduinoPinDescription[]
} PwmMappingType;

typedef struct _DigitalMappingType
{
    LPC_GPIO_T *pPORT;
    // IOCON register value to connect and configure GPIO function to it's pin designated in g_ArduinoPinDescription[]
    uint32_t modefunc:16;
    uint32_t modefuncPullup:16;
    uint32_t modefuncPulldown:16;
    uint32_t reserved:16;
} DigitalMappingType;

typedef struct _LPCPinDescription
{
    uint32_t port:8;
    uint32_t pin:8;
    uint32_t modefunc:16; // IOCON register value used to initialize pins at startup
    AnalogChannelType adcChannel; // Analog input in Arduino context: (0-5, 0-7 Mini & Nano, 0-15 Mega)
    PwmChannelType pwmChannel; // Which LPC PWM channel (if any) is configured on this pin
    ExtInterruptType intPin; // to which pin interrupt channel is this pin assigned (attachInterrupt/detachInterrupt)
} LPCPinDescription;

#ifdef __cplusplus
extern "C" {
#endif

extern const LPCPinDescription g_ArduinoPinDescription[];
extern const PwmMappingType g_ArduinoPinMappingPwm[];
extern const DigitalMappingType g_ArduinoPinMappingDigital[];
extern const uint32_t arduinoPinsArrayLength;

// Shorthand definitions
#define APIN_PORT(p)  (g_ArduinoPinDescription[p].port)
#define APIN_PIN(p)   (g_ArduinoPinDescription[p].pin)
#define APIN_MODE(p)  (g_ArduinoPinDescription[p].modefunc)
#define APIN_ADC(p)   (g_ArduinoPinDescription[p].adcChannel)
#define APIN_PWM(p)   (g_ArduinoPinDescription[p].pwmChannel)
#define APIN_INT(p)   (g_ArduinoPinDescription[p].intPin)

#define PWM_MAP_TIMER(p)    (g_ArduinoPinMappingPwm[p].pTMR)
#define PWM_MAP_CHANNEL(p)  (g_ArduinoPinMappingPwm[p].channel)
#define PWM_MAP_MODE(p)     (g_ArduinoPinMappingPwm[p].modefunc)

#define DIG_MAP_PORT(p)     (g_ArduinoPinMappingDigital[p].pPORT)
#define DIG_MAP_MODE(p)     (g_ArduinoPinMappingDigital[p].modefunc)
#define DIG_MAP_PUP(p)      (g_ArduinoPinMappingDigital[p].modefuncPullup)
#define DIG_MAP_PDOWN(p)    (g_ArduinoPinMappingDigital[p].modefuncPulldown)

#ifdef __cplusplus
}
#endif

#endif
