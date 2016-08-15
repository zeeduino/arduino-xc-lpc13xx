/*
  board_serial.h - Arduino-XC HAL API implementation of serial comm
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

#include "lib/UARTSerial.h"
#include "board/board_serial.h"
#include "ringbuffer.h"
#include "board/variant.h"

#include "chip.h"

// ----------------------------------------------------------------------------
//   CONSTANT DEFINITIONS
// ----------------------------------------------------------------------------
// These "constants" are declared as variables and their value is platform specific
// This way we can declare these in code that is not platform specific (i.e. core or lib)
// and then define their values here in platform specific part where we need to access
// platform specific values (like UART_LCR_WLEN8, UART_LCR_PARITY_F_0, etc.)
extern const uint32_t SERIAL_8N1 = (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS);
extern const uint32_t SERIAL_8E1 = (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_EVEN | UART_LCR_PARITY_EN);
extern const uint32_t SERIAL_8O1 = (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_ODD | UART_LCR_PARITY_EN);
extern const uint32_t SERIAL_8M1 = (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_F_1 | UART_LCR_PARITY_EN);
extern const uint32_t SERIAL_8S1 = (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_F_0 | UART_LCR_PARITY_EN);

/******************************************************************
 * UART objects
 */
/*
 * For each USART present on the chip, we declare one context
 */
Serial_UART_Context UART0_Context;
//Serial_UART_Context UART1_Context;
//Serial_UART_Context UART2_Context;

// This can be either declared like this...
//UARTClass Serial(LPC_USART, UART0_IRQn, 0, &rxring, &txring);
// ...or like this...
UARTSerial Serial;
// and then constructed in Serial_Init()

/* Transmit and receive ring buffer sizes */
#define UART_TXRB_SIZE 128   /* Send */
#define UART_RXRB_SIZE 32    /* Receive */

// ----------------------------------------------------------------------------
//   INIT/DEINIT FUNCTIONS
// ----------------------------------------------------------------------------
void Serial_Init(void)
{
	// Add code here for each USART chip has and you want to use in Zeeduino
	UART0_Context.dwId = 0;
	UART0_Context.pUART = (void*)LPC_USART;
	UART0_Context.dwIrq = UART0_IRQn;
	UART0_Context.rx_rbr = new RingBuffer;
	UART0_Context.tx_rbr = new RingBuffer;
	UART0_Context.rx_buffer = new uint8_t[UART_RXRB_SIZE];
	UART0_Context.tx_buffer = new uint8_t[UART_TXRB_SIZE];
	RingBuffer_init(UART0_Context.rx_rbr, UART0_Context.rx_buffer, UART_RXRB_SIZE, sizeof(uint8_t));
	RingBuffer_init(UART0_Context.tx_rbr, UART0_Context.tx_buffer, UART_TXRB_SIZE, sizeof(uint8_t));
    Serial = UARTSerial(&UART0_Context);
}

// ----------------------------------------------------------------------------
void Serial_UART_Init(void *uartHandle, const uint32_t dwBaudRate, const uint32_t configData)
{
    LPC_USART_T* pUart = (LPC_USART_T*)UART_CONTEXT(uartHandle)->pUART;
    IRQn_Type _dwIrq = (IRQn_Type)UART_CONTEXT(uartHandle)->dwIrq;

    Chip_UART_Init(pUart);
    Chip_UART_SetBaud(pUart, dwBaudRate);
    Chip_UART_ConfigData(pUart, configData);
    Chip_UART_SetupFIFOS(pUart, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
    Chip_UART_TXEnable(pUart);

    /* Enable receive data and line status interrupt, the transmit interrupt
       is handled by the driver. */
    Chip_UART_IntEnable(pUart, (UART_IER_RBRINT | UART_IER_RLSINT));

    NVIC_SetPriority(_dwIrq, 1);
    NVIC_EnableIRQ(_dwIrq);
}

// ----------------------------------------------------------------------------
void Serial_UART_End(void *uartHandle)
{
    LPC_USART_T* pUart = (LPC_USART_T*)UART_CONTEXT(uartHandle)->pUART;
    IRQn_Type _dwIrq = (IRQn_Type)UART_CONTEXT(uartHandle)->dwIrq;

    NVIC_DisableIRQ(_dwIrq);
    Chip_UART_DeInit(pUart);
}

// ----------------------------------------------------------------------------
void Serial_UART_Flush(void *uartHandle)
{
    // Wait for transmission to complete
    while (!Serial_UART_TxRegisterEmpty(uartHandle)) yield();
}

// ----------------------------------------------------------------------------
//   LOW-LEVEL RX/TX FUNCTIONS, ALSO USED IN IRQ HANDLER
// ----------------------------------------------------------------------------
/* UART transmit-only interrupt handler for ring buffers */
void Serial_UART_Transmit(void *uartHandle)
{
	uint8_t ch;
    LPC_USART_T* pUart = (LPC_USART_T*)UART_CONTEXT(uartHandle)->pUART;

    /* Fill FIFO until full or until TX ring buffer is empty */
    while (Serial_UART_TxRegisterEmpty(uartHandle) && RingBuffer_popOne(UART_CONTEXT(uartHandle)->tx_rbr, &ch))
    {
        Chip_UART_SendByte(pUart, ch);
    }
}

// ----------------------------------------------------------------------------
/* UART receive-only interrupt handler for ring buffers */
void Serial_UART_Receive(void *uartHandle)
{
    LPC_USART_T* pUart = (LPC_USART_T*)UART_CONTEXT(uartHandle)->pUART;

    /* New data will be ignored if data not popped in time */
    while (Serial_UART_RxRegisterHasData(uartHandle))
    {
        uint8_t ch = Chip_UART_ReadByte(pUart);
        RingBuffer_pushOne(UART_CONTEXT(uartHandle)->rx_rbr, &ch);
    }
}

// ----------------------------------------------------------------------------
/*
 * When this returns true, that means UART is ready for the next byte to be sent.
 * This usually means: the top byte of the hardware UART FIFO is available
 * to accept next byte to be sent.
 */
bool Serial_UART_TxRegisterEmpty(void *uartHandle)
{
    LPC_USART_T* pUart = (LPC_USART_T*)UART_CONTEXT(uartHandle)->pUART;
	return (Chip_UART_ReadLineStatus(pUart) & UART_LSR_THRE) != 0;
}

// ----------------------------------------------------------------------------
/*
 * When this returns true, that means UART has received a byte and it is waiting
 * for us to read it. This usually means: location at which UART register that
 * points to the oldest received byte in hardware UART FIFO, has not been read yet,
 * there's still data to read from receive FIFO.
 */
bool Serial_UART_RxRegisterHasData(void *uartHandle)
{
    LPC_USART_T* pUart = (LPC_USART_T*)UART_CONTEXT(uartHandle)->pUART;
	return (Chip_UART_ReadLineStatus(pUart) & UART_LSR_RDR) != 0;
}

// ----------------------------------------------------------------------------
//   INTERRUPTS
// ----------------------------------------------------------------------------
void Serial_UART_Set_Interrupt_Priority(void *uartHandle, uint32_t priority)
{
	IRQn_Type _dwIrq = (IRQn_Type)UART_CONTEXT(uartHandle)->dwIrq;

    NVIC_SetPriority(_dwIrq, priority & 0x0F);
}

// ----------------------------------------------------------------------------
uint32_t Serial_UART_Get_Interrupt_Priority(void *uartHandle)
{
	IRQn_Type _dwIrq = (IRQn_Type)UART_CONTEXT(uartHandle)->dwIrq;

    return NVIC_GetPriority(_dwIrq);
}

// ----------------------------------------------------------------------------
void Serial_UART_Disable_Interrupt(void *uartHandle)
{
    LPC_USART_T* pUart = (LPC_USART_T*)UART_CONTEXT(uartHandle)->pUART;
	Chip_UART_IntDisable(pUart, UART_IER_THREINT);
}

// ----------------------------------------------------------------------------
void Serial_UART_Enable_Interrupt(void *uartHandle)
{
    LPC_USART_T* pUart = (LPC_USART_T*)UART_CONTEXT(uartHandle)->pUART;
	Chip_UART_IntEnable(pUart, UART_IER_THREINT);
}

// ----------------------------------------------------------------------------
//   EVENTS
// ----------------------------------------------------------------------------
// This fires in main loop whenever UART receives any bytes
// and they are available to be read
void serialEvent() __attribute__((weak));
void serialEvent() { }
// This one is called by main.cpp after every loop() call
void serialEventRun(void)
{
  if (Serial.available()) serialEvent();
}

// ----------------------------------------------------------------------------
//   IRQ HANDLERS
// ----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

/**
 * All LPC IRQ handlers and such have to be implemented here
 * within extern "C" declaration, otherwise their names will get mangled
 * by C++ compiler and they WON'T get linked where you would expect them
 * to be because they are declared with __attribute__((weak)).
 */

/**
 * Each USART peripheral will have it's own IRQ handler.
 * In the IRQ code, use a corresponding UARTx_Context declared above,
 * similar to what we did here for UART0 (the only USART on LPC1347)
 */
#if defined (__USE_LPCOPEN)
void UART_IRQHandler(void)
#else
USART_IRQHandler
#endif
{
    /* Want to handle any errors? Do it here. */

    /* Use default ring buffer handler. Override this with your own
       code if you need more capability. */
    /* Handle transmit interrupt if enabled */
    if (LPC_USART->IER & UART_IER_THREINT)
    {
    	Serial_UART_Transmit(&UART0_Context);

        /* Disable transmit interrupt if the ring buffer is empty */
        if (RingBuffer_isEmpty(UART0_Context.tx_rbr))
        {
        	Serial_UART_Disable_Interrupt(&UART0_Context);
        }
    }

    /* Handle receive interrupt */
    Serial_UART_Receive(&UART0_Context);
}


#ifdef __cplusplus
}
#endif


