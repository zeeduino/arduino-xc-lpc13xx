/*
 * usb_device_lpc1347.h
 *
 *  Created on: Aug 11, 2016
 *      Author: iceman
 */

#ifndef USB_DEVICE_LPC1347_H_
#define USB_DEVICE_LPC1347_H_

#include <inttypes.h>
#include "usbd_rom_api.h"

/**************************************************************************/
/* Board USB HAL internal variables */
extern uint32_t membase;
extern uint32_t memsize;
extern USBD_HANDLE_T g_hUsb;

/**************************************************************************/
/* Board USB HAL internal functions */
ErrorCode_t __USB_Device_Custom_onConfigured (USBD_HANDLE_T hUsb);
ErrorCode_t __USB_Device_Custom_Init (void);

ErrorCode_t __USB_Device_CDC_onConfigured(USBD_HANDLE_T hUsb);
ErrorCode_t __USB_Device_CDC_Init(void);


#endif /* USB_DEVICE_LPC1347_H_ */
