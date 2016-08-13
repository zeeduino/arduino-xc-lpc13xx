/*
 * board_usb_device_custom.c
 *
 *  Created on: Aug 13, 2016
 *      Author: iceman
 */

#include <string.h>
#include <stdbool.h>

#include "usbd_rom_api.h"

#include "board/board_usb_device.h"
#include "usb_device_descriptors.h"
#include "usb_device_lpc1347.h"

#include "tracer.h"


/**************************************************************************/
/* Private variables */
static volatile bool isCustomEpInReady = false;


/**************************************************************************/
/* Board API implementation */
bool Board_USB_Device_Custom_isReadyToSend(void)
{
  return Board_USB_Device_isConfigured() && isCustomEpInReady;
}

ErrorCode_t Board_USB_Device_Custom_Send(uint8_t const * p_data, uint32_t length)
{
	if(p_data == NULL || length == 0 || !Board_USB_Device_Custom_isReadyToSend()) return false;

  uint32_t written_length = USBD_API->hw->WriteEP(g_hUsb, CUSTOM_EP_IN, (uint8_t*) p_data, length);
  if ( written_length != length)
  {
    return ERR_FAILED;
  }
  isCustomEpInReady = false;

  return LPC_OK;
}

bool Board_USB_Device_Custom_Receive(uint8_t * p_data, uint32_t length)
{
	return true;
}

/**************************************************************************/
/* Private functions */
//static ErrorCode_t __USB_Device_Custom_Control_Endpoint_ISR(USBD_HANDLE_T hUsb, void* data, uint32_t event)
//{
//  return LPC_OK;
//}

static ErrorCode_t __USB_Device_Custom_Bulk_Endpoint_In_ISR (USBD_HANDLE_T husb, void* data, uint32_t event)
{
  if (USB_EVT_IN == event)
  {
    isCustomEpInReady = true;
  }

  return LPC_OK;
}

static void Board_USB_Device_Custom_Received_ISR(uint8_t * p_buffer, uint32_t length)
{
	// RingBuffer_Insert(&ff_prot_cmd, p_data);
}

static ErrorCode_t __USB_Device_Custom_Bulk_Endpoint_Out_ISR (USBD_HANDLE_T husb, void* data, uint32_t event)
{
  if (USB_EVT_OUT == event)
  {
    uint8_t buffer[64] = { 0 }; // size is 64
    uint32_t length = USBD_API->hw->ReadEP(husb, CUSTOM_EP_OUT, buffer);
   	Board_USB_Device_Custom_Received_ISR( buffer, length);
  }
  return LPC_OK;
}

/**************************************************************************/
/* Board HAL internal functions */
ErrorCode_t __USB_Device_Custom_Init ()
{
	ErrorCode_t ret = LPC_OK;

// (USB_INTERFACE_DESCRIPTOR const *) pCustomIntfDesc = &USB_FsConfigDescriptor.Custom_Interface;

//  USBD_API->core->RegisterClassHandler(g_hUsb, __USB_Device_Custom_Control_Endpoint_ISR, NULL);

  ret = USBD_API->core->RegisterEpHandler (g_hUsb, ((CUSTOM_EP_IN & 0x0F) << 1) +1, __USB_Device_Custom_Bulk_Endpoint_In_ISR , NULL);
  if(ret != LPC_OK)
	   return ret;

  ret = USBD_API->core->RegisterEpHandler (g_hUsb, (CUSTOM_EP_OUT & 0x0F) << 1 , __USB_Device_Custom_Bulk_Endpoint_Out_ISR, NULL);
  if(ret != LPC_OK)
	   return ret;

  ADD_EVENT(EVENT_CUSTOM_INIT)

  return LPC_OK;
}

ErrorCode_t __USB_Device_Custom_onConfigured (USBD_HANDLE_T hUsb)
{
  isCustomEpInReady = true;

  return LPC_OK;
}

