/*
 * Until hard fault issue(s) are solved, USB device code will be "C"
 * and here we place all related to USB device that has to be "C++".
 */

#include "lib/USBDeviceCDC.h"
#include "board/board_serial_context.h"

extern "C" Serial_UART_Context USBUART_Context;

USBDeviceCDC UsbSerial(&USBUART_Context);
