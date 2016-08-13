/*
 * board_usb_device.c
 *
 *  Created on: Aug 13, 2016
 *      Author: iceman
 */


#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "usb_device_config.h"
#include "board/board_usb_device.h"
#include "usb_device_descriptors.h"
#include "usb_device_lpc1347.h"

#include "board/board_usb_cdc.h"
#include "board/board_usb_custom.h"

#include "chip.h"
//#include "flash_13xx.h"
#include "usbd_rom_api.h"

#include "tracer.h"

/**************************************************************************/
/* Board USB HAL internal variables */
USBD_HANDLE_T g_hUsb;

/* LPC1347 */
#define USB_STACK_MEM_BASE      0x20004800
#define USB_STACK_MEM_SIZE      0x0800
uint32_t membase;
uint32_t memsize;

const  USBD_API_T *g_pUsbApi;

/**************************************************************************/
/* local variables */

volatile static bool isConfigured = false;
volatile static bool isInitialized = false;


void USB_IRQHandler(void)
{
	uint32_t *addr = (uint32_t *) LPC_USB->EPLISTSTART;

	/*	WORKAROUND for artf32289 ROM driver BUG:
	    As part of USB specification the device should respond
	    with STALL condition for any unsupported setup packet. The host will send
	    new setup packet/request on seeing STALL condition for EP0 instead of sending
	    a clear STALL request. Current driver in ROM doesn't clear the STALL
	    condition on new setup packet which should be fixed.
	 */
	if ( LPC_USB->DEVCMDSTAT & _BIT(8) ) {	/* if setup packet is received */
		addr[0] &= ~(_BIT(29));	/* clear EP0_OUT stall */
		addr[2] &= ~(_BIT(29));	/* clear EP0_IN stall */
	}

	USBD_API->hw->ISR(g_hUsb);
}

/**************************************************************************/
/* Private functions */

static ErrorCode_t __USB_Device_Configure_Event (USBD_HANDLE_T hUsb)
{
  USB_CORE_CTRL_T* pCtrl = (USB_CORE_CTRL_T*)hUsb;
  if (pCtrl->config_value)
  {
    #ifdef CONFIG_USB_DEVICE_HID
    __USB_Device_HID_onConfigured(hUsb);
    #endif

    #ifdef CONFIG_USB_DEVICE_CDC
    __USB_Device_CDC_onConfigured(hUsb);
    #endif

    #ifdef CONFIG_USB_DEVICE_MSC
    __USB_Device_MSC_onConfigured(hUsb);
    #endif

    #ifdef CONFIG_USB_DEVICE_CUSTOM_CLASS
    __USB_Device_Custom_onConfigured(hUsb);
    #endif
  }

  isConfigured = true;
  ADD_EVENT(EVENT_DEV_CONFIGURED)

  return LPC_OK;
}

static ErrorCode_t __USB_Device_Reset_Event (USBD_HANDLE_T hUsb)
{
  isConfigured = false;
  ADD_EVENT(EVENT_DEV_RESET)
  return LPC_OK;
}

static void __USB_Device_Chip_Init()
{
	/* enable clocks and pinmux */
	Chip_USB_Init();
	/* Enable AHB clock to the USB block and USB RAM. */
	LPC_SYSCTL->SYSAHBCLKCTRL |= ((0x1 << 14) | (0x1 << 27));
	// Now we can set USB stack RAM base address to be in the USB RAM space
	/* Pull-down is needed, or internally, VBUS will be floating. This is to
	 address the wrong status in VBUSDebouncing bit in CmdStatus register.  */
	LPC_IOCON->PIO0[3] &= ~0x1F;
	LPC_IOCON->PIO0[3] |= (0x01 << 0); /* Secondary function VBUS */
	LPC_IOCON->PIO0[6] &= ~0x07;
	LPC_IOCON->PIO0[6] |= (0x01 << 0); /* Secondary function SoftConn */
}

static void __USB_Device_SetSerialNumber()
{
	int i;
	/* Use the 128-bit chip ID for USB serial to make sure it's unique */
	FLASH_READ_UID_OUTPUT_T uidOutput;
	Chip_FLASH_ReadUID(&uidOutput);
	//	  iapReadUID(uid);  /* 1st byte is LSB, 4th byte is MSB */
	uint32_t* uid = uidOutput.id;

	// This creates a 32-character long ASCII string at the start of USB_StringDescriptor_Collection.strSerial buffer
	sprintf((char*) USB_StringDescriptor_Collection.strSerial,
			"%08X%08X%08X%08X", (unsigned int) uid[3], (unsigned int) uid[2],
			(unsigned int) uid[1], (unsigned int) uid[0]);

	// This converts ASCII string inplace into Unicode by using casting from uint8_t to uint16_t
	// We can do this because USB_StringDescriptor_Collection.strSerial array is array of 2-byte elements
	// and sprintf used only bottom half of the buffer for serial bumber
	for (i = USB_STRING_SERIAL_LEN - 1; i > 0; i--) {
		USB_StringDescriptor_Collection.strSerial[i] = ((uint8_t*) USB_StringDescriptor_Collection.strSerial)[i];
		((uint8_t*) USB_StringDescriptor_Collection.strSerial)[i] = 0;
	}
}

static void __USB_Device_InitStringDescriptors()
{
int i;
/*
 * There's implicit converstion here from char (uint8_t) in CONFIG_USB_DEVICE_STRING_* arrays
 * to uint16_t in USB_StringDescriptor_Collection.str* arrays. This effectively converts our
 * ASCII representation of device strings in CONFIG_USB_DEVICE_STRING_* arrays into Unicode
 */
	  for (i=0; i < strlen(CONFIG_USB_DEVICE_STRING_MANUFACTURER); i++)
		  USB_StringDescriptor_Collection.strManufacturer[i] = CONFIG_USB_DEVICE_STRING_MANUFACTURER[i];

	  for (i=0; i < strlen(CONFIG_USB_DEVICE_STRING_PRODUCT); i++)
		  USB_StringDescriptor_Collection.strProduct[i] = CONFIG_USB_DEVICE_STRING_PRODUCT[i];

	__USB_Device_SetSerialNumber();
}

/**************************************************************************/
/* Board API implementation */
bool Board_USB_Device_isConfigured(void)
{
  return isConfigured;
}

bool Board_USB_Device_Init(void)
{
	USBD_API_INIT_PARAM_T usb_param;
	USB_CORE_DESCS_T core_descriptors;

	if(isInitialized) return true;

	__USB_Device_InitStringDescriptors();

	__USB_Device_Chip_Init();

	membase = USB_STACK_MEM_BASE;
	memsize = USB_STACK_MEM_SIZE;

	/* initialize USBD ROM API pointer. */
	g_pUsbApi = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

	/* initialize call back structures */
	memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
	usb_param.usb_reg_base = LPC_USB0_BASE;
	/*	WORKAROUND for artf44835 ROM driver BUG:
	    Code clearing STALL bits in endpoint reset routine corrupts memory area
	    next to the endpoint control data. For example When EP0, EP1_IN, EP1_OUT,
	    EP2_IN are used we need to specify 3 here. But as a workaround for this
	    issue specify 4. So that extra EPs control structure acts as padding buffer
	    to avoid data corruption. Corruption of padding memory doesn’t affect the
	    stack/program behaviour.
	 */
	usb_param.max_num_ep = 4 + 1;
	usb_param.mem_base = membase;
	usb_param.mem_size = memsize;

	usb_param.USB_Configure_Event = __USB_Device_Configure_Event;
	usb_param.USB_Reset_Event     = __USB_Device_Reset_Event;


	/* Set the USB descriptors */
	core_descriptors.device_desc = (uint8_t *) &USB_DeviceDescriptor;
	core_descriptors.string_desc = (uint8_t *) &USB_StringDescriptor_Collection;
	/* Note, to pass USBCV test full-speed only devices should have both
	 * descriptor arrays point to same location and device_qualifier set
	 * to 0.
	 */
	core_descriptors.high_speed_desc = (uint8_t *) &USB_FsConfigDescriptor;
	core_descriptors.full_speed_desc = (uint8_t *) &USB_FsConfigDescriptor;
	core_descriptors.device_qualifier = 0;

	ErrorCode_t ret = LPC_OK;
	/* USB Initialization */
	ret = USBD_API->hw->Init(&g_hUsb, &core_descriptors, &usb_param);
	if (ret == LPC_OK)
	{
		  membase += (memsize - usb_param.mem_size);
		  memsize = usb_param.mem_size;

		  /* Initialise the class driver(s) */
		  #ifdef CONFIG_USB_DEVICE_CDC
		    ret = __USB_Device_CDC_Init();
		    if(ret != LPC_OK) return false;
		  #endif

#ifdef CONFIG_USB_DEVICE_HID_KEYBOARD
  ret = __USB_Device_HID_Init(g_hUsb , &USB_FsConfigDescriptor.HID_KeyboardInterface ,
          HID_KeyboardReportDescriptor, USB_FsConfigDescriptor.HID_KeyboardHID.DescriptorList[0].wDescriptorLength,
          &membase , &memsize);
  if(ret != LPC_OK) return false;
#endif

#ifdef CONFIG_USB_DEVICE_HID_MOUSE
  ret = __USB_Device_HID_Init(g_hUsb , &USB_FsConfigDescriptor.HID_MouseInterface    ,
          HID_MouseReportDescriptor, USB_FsConfigDescriptor.HID_MouseHID.DescriptorList[0].wDescriptorLength,
          &membase , &memsize);
  if(ret != LPC_OK) return false;
#endif

#ifdef CONFIG_USB_DEVICE_HID_GENERIC
  ret = __USB_Device_HID_Init(g_hUsb , &USB_FsConfigDescriptor.HID_GenericInterface    ,
          HID_GenericReportDescriptor, USB_FsConfigDescriptor.HID_GenericHID.DescriptorList[0].wDescriptorLength,
          &membase , &memsize);
  if(ret != LPC_OK) return false;
#endif

#ifdef CONFIG_USB_DEVICE_MSC
  // there is chance where SD card is not inserted, thus the msc init fails, should continue instead of return
  ret = __USB_Device_MSC_Init(g_hUsb, &USB_FsConfigDescriptor.MSC_Interface, &membase, &memsize);
	  if(ret != LPC_OK) return false;
#endif

		#ifdef CONFIG_USB_DEVICE_CUSTOM_CLASS
		  ret = __USB_Device_Custom_Init();
		    if(ret != LPC_OK) return false;
		#endif

	  if (ret == LPC_OK)
	  {
			NVIC_EnableIRQ(USB0_IRQn);
			USBD_API->hw->Connect(g_hUsb, 1);

			isInitialized = true;
	   }
	}

  return true;
}



