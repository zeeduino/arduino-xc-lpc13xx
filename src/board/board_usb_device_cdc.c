/*
 * board_usb_device_cdc.c
 *
 *  Created on: Aug 13, 2016
 *      Author: iceman
 */

#include <string.h>
#include <stdbool.h>

#include "usbd_rom_api.h"

#include "ringbuffer.h"
#include "board/board_usb_device.h"
#include "usb_device_descriptors.h"
#include "usb_device_lpc1347.h"

#define WEAK __attribute__ ((weak))
#define ALIAS(f) __attribute__ ((weak, alias (#f)))

static USBD_HANDLE_T g_hCdc;
static CDC_LINE_CODING line_coding;
static bool isConnected = false;             /* ToDo: Consider work-around */

#include "tracer.h"


//============================================================================
/* Transmit and receive ring buffers */
static RingBuffer txring, rxring;

/* Transmit and receive ring buffer sizes */
#define UART_TXRB_SIZE (8*CDC_DATA_EP_MAXPACKET_SIZE)	/* Send */
#define UART_RXRB_SIZE (2*CDC_DATA_EP_MAXPACKET_SIZE)	/* Receive */

/* Transmit and receive buffers */
static uint8_t rxbuff[UART_RXRB_SIZE], txbuff[UART_TXRB_SIZE];
//============================================================================

WEAK void USB_Device_CDC_Receive_ISR(void);

/**************************************************************************/
/*!
    @brief  Stub for the optional CDC receive ISR that can be used
            to perform some action when data arrives via USB CDC
*/
/**************************************************************************/
void USB_Device_CDC_Receive_ISR (void)
{
  return;
}

/**************************************************************************/
/* Board API implementation */
bool Board_USB_Device_CDC_isConnected(void)
{
  return isConnected;
}

bool Board_USB_Device_CDC_putc(uint8_t c)
{
  if ( !RingBuffer_pushOne(&txring, &c) )
  {
      return false;
  }

  return true;
}

bool Board_USB_Device_CDC_getc(uint8_t *c)
{
	if(!c) return false; /* Make sure pointer isn't NULL */

	return RingBuffer_popOne(&rxring, c);
}

uint16_t Board_USB_Device_CDC_Send(uint8_t* buffer, uint16_t count)
{
  uint16_t i=0;

  if( !(buffer && count) ) return 0;  //buffer !=NULL & count != 0

  while (i < count && Board_USB_Device_CDC_putc(buffer[i]) )
  {
    i++;
  }

  return i;
}

uint16_t Board_USB_Device_CDC_Receive(uint8_t* buffer, uint16_t max)
{
  if( !(buffer && max) ) return 0; // buffer not NULL and max != 0

  return RingBuffer_pop(&rxring, buffer, max);
}

/**************************************************************************/
/* Private functions */
static ErrorCode_t __USB_Device_CDC_SetLineCoding_Event(USBD_HANDLE_T hUsb, CDC_LINE_CODING *lineCoding)
{
  if( !lineCoding ) return ERR_FAILED;

  memcpy(&line_coding, lineCoding, sizeof(CDC_LINE_CODING));

  // when terminal on host connects to VCOM port this event is fired
  // so we assume we are connected to terminal here
  isConnected = true;

  ADD_EVENT(EVENT_CDC_LINECODE)

  return LPC_OK;
}

static ErrorCode_t __USB_Device_CDC_SendBreak_Event(USBD_HANDLE_T hCDC, uint16_t mstime)
{
  return LPC_OK;
}

static ErrorCode_t __USB_Device_CDC_EPIn_Bulk_Handler(USBD_HANDLE_T hUsb, void* data, uint32_t event)
{
  if (USB_EVT_IN == event)
  {
    uint8_t buffer[CDC_DATA_EP_MAXPACKET_SIZE];
    uint16_t count;

    count = RingBuffer_pop(&txring, buffer, CDC_DATA_EP_MAXPACKET_SIZE);
    USBD_API->hw->WriteEP(hUsb, CDC_DATA_EP_IN, buffer, count); // write data to EP

//    isConnected = true;
  }

  return LPC_OK;
}

static ErrorCode_t __USB_Device_CDC_EPOut_Bulk_Handler(USBD_HANDLE_T hUsb, void* data, uint32_t event)
{
  if (USB_EVT_OUT == event)
  {
    uint16_t count, i;
    uint8_t buffer[CDC_DATA_EP_MAXPACKET_SIZE];

    count = USBD_API->hw->ReadEP(hUsb, CDC_DATA_EP_OUT, buffer);
    for (i=0; i<count; i++)
    {
    	RingBuffer_pushOne(&rxring, buffer+i);
    }

//    isConnected = true;

    USB_Device_CDC_Receive_ISR();
  }

  return LPC_OK;
}

/**************************************************************************/
/* Board HAL internal functions */
ErrorCode_t __USB_Device_CDC_onConfigured(USBD_HANDLE_T hUsb)
{
  uint8_t dummy=0;
  USBD_API->hw->WriteEP(hUsb, CDC_DATA_EP_IN, &dummy, 1); // initial packet for IN endpoint, will not work if omitted

//  isConnected = true;

  RingBuffer_flush(&rxring);
  RingBuffer_flush(&txring);

  ADD_EVENT(EVENT_CDC_CONFIGURED)

  return LPC_OK;
}

ErrorCode_t __USB_Device_CDC_Init()
{
	ErrorCode_t ret = LPC_OK;

//	if( !(pControlIntfDesc && pDataIntfDesc) ) return ERR_FAILED;

	USB_INTERFACE_DESCRIPTOR const *const pControlIntfDesc = &USB_FsConfigDescriptor.CDC_CCI_Interface;
	USB_INTERFACE_DESCRIPTOR const *const pDataIntfDesc = &USB_FsConfigDescriptor.CDC_DCI_Interface;

	RingBuffer_init(&rxring, rxbuff, UART_RXRB_SIZE, sizeof(rxbuff[0]));
	RingBuffer_init(&txring, txbuff, UART_TXRB_SIZE, sizeof(txbuff[0]));


	USBD_CDC_INIT_PARAM_T cdc_param =
  {
    .mem_base      = membase,
    .mem_size      = memsize,

    .cif_intf_desc = (uint8_t*) pControlIntfDesc,
    .dif_intf_desc = (uint8_t*) pDataIntfDesc,

    .SetLineCode   = __USB_Device_CDC_SetLineCoding_Event,
    .SendBreak     = __USB_Device_CDC_SendBreak_Event,

    // .CIC_GetRequest   = CDC_Control_GetRequest, // bug from romdrive cannot hook to this handler
    // Bug from ROM driver: can not hook bulk in & out handler here, must use USBD API register instead
    // .CDC_BulkIN_Hdlr  = CDC_BulkIn_Hdlr,
    // .CDC_BulkOUT_Hdlr = CDC_BulkOut_Hdlr,
  };

	 ret = USBD_API->core->RegisterEpHandler (g_hUsb , ((CDC_DATA_EP_IN & 0x0F) << 1) +1 , __USB_Device_CDC_EPIn_Bulk_Handler  , NULL);
	 if(ret != LPC_OK) return ret;

	 ret = USBD_API->core->RegisterEpHandler (g_hUsb , (CDC_DATA_EP_OUT & 0x0F) << 1     , __USB_Device_CDC_EPOut_Bulk_Handler , NULL);
	 if(ret != LPC_OK) return ret;

	 ret = USBD_API->cdc->init(g_hUsb, &cdc_param, &g_hCdc);
	 if(ret != LPC_OK) return ret;

//  if(*mem_size < cdc_param.mem_size) return ERR_FAILED; // "not enough memory"

  membase += (memsize - cdc_param.mem_size);
  memsize = cdc_param.mem_size;

  isConnected = false;

  return LPC_OK;
}

