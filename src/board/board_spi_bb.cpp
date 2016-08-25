/*
  board_spi_bb.cpp - Arduino-XC HAL API implementation of SPI peripheral - bit-bang/software version
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


#include "board/variant.h"
#include "lib/SPI.h"
#include "wiring_delay.h"
#include "pins_arduino.h"
#include "board/board_spi.h"
#include "board/board_fast_pins.h"

#include "chip.h"

// ~2.27kHz : ~220us data pulse width -> ~440us bit width -> ~2.27kHz
//#define BB_DELAY() delayMicroseconds(50)
// ~12kHz : 40us data pulse width -> 80us bit width -> ~12.5kHz
//#define BB_DELAY() delayMicroseconds(5)
// ~32kHz : 15us data pulse width -> 30us bit width -> ~33kHz
#define BB_DELAY() delayMicroseconds(0)
// as fast as we can
// ~196kHz : 2.6us data pulse width -> 5.2us bit width -> ~192kHz
//#define BB_DELAY()

typedef struct _BoardSPIPortInternal
{
	FAST_ZPIN_HANDLE(CS)
	FAST_ZPIN_HANDLE(SCK)
	FAST_ZPIN_HANDLE(MISO)
	FAST_ZPIN_HANDLE(MOSI)
} BoardSPIPortInternal;


BoardSPIPortInternal onlyCurrentSPIPort;

BoardSPIPortInternal *currentPort = &onlyCurrentSPIPort;

void __init_spi_handle(BoardSPIPort *port)
{
	port->handle = (void*)&onlyCurrentSPIPort;
	currentPort = &onlyCurrentSPIPort;

	FAST_ZPIN_INIT(currentPort->CS, port->cs);
	FAST_ZPIN_INIT(currentPort->SCK, port->clk);
	FAST_ZPIN_INIT(currentPort->MISO, port->miso);
	FAST_ZPIN_INIT(currentPort->MOSI, port->mosi);
}

void Board_SPI_SetInternalPortStateDefault(BoardSPIPort *port)
{
	FAST_ZPIN_INIT(currentPort->CS, SPI_BUILTIN_SSEL);
	FAST_ZPIN_INIT(currentPort->SCK, SPI_BUILTIN_SCK);
	FAST_ZPIN_INIT(currentPort->MISO, SPI_BUILTIN_MISO);
	FAST_ZPIN_INIT(currentPort->MOSI, SPI_BUILTIN_MOSI);
}

void Board_SPI_SetInternalPortState(BoardSPIPort *port)
{
	currentPort = (BoardSPIPortInternal *)(port->handle);
}

uint8_t Board_SPI_Transfer_MSB(BoardSPIPort *port, uint8_t data)
{
    uint8_t rx;
    int i;

    Board_SPI_SetInternalPortState(port);

//    FAST_ZPIN_LOW(currentPort->CS);
    BB_DELAY();
    rx = 0x00;
    for (i = 7; i >= 0; i--)
    {
        rx <<= 1;

        FAST_ZPIN_LOW(currentPort->SCK);
        BB_DELAY();

        FAST_ZPIN_WRITE(currentPort->MOSI, data & (1 << i));
        BB_DELAY();

        FAST_ZPIN_HIGH(currentPort->SCK);
        BB_DELAY();
        if (FAST_ZPIN_READ(currentPort->MISO))
            rx |= 1;
        BB_DELAY();
    }
//    FAST_ZPIN_WRITE(currentPort->MOSI, true); // MOSI HIGH when idle

    return rx;
}

uint8_t Board_SPI_Transfer_LSB(BoardSPIPort *port, uint8_t data)
{
    uint8_t rx;
    int i;

    Board_SPI_SetInternalPortState(port);

//    FAST_ZPIN_LOW(currentPort->CS);
    BB_DELAY();
    rx = 0x00;
    for (i = 0; i <= 7; i++)
    {
        rx <<= 1;

        FAST_ZPIN_LOW(currentPort->SCK);
        BB_DELAY();

        FAST_ZPIN_WRITE(currentPort->MOSI, data & (1 << i));
        BB_DELAY();

        FAST_ZPIN_HIGH(currentPort->SCK);
        BB_DELAY();
        if (FAST_ZPIN_READ(currentPort->MISO))
            rx |= 1;
        BB_DELAY();
    }
//    FAST_ZPIN_WRITE(currentPort->MOSI, true); // MOSI HIGH when idle

    return rx;
}

uint8_t Board_SPI_Transfer(BoardSPIPort *port, uint8_t data)
{
	return Board_SPI_Transfer_MSB(port, data);
}

void Board_SPI_CS_High(BoardSPIPort *port)
{
    Board_SPI_SetInternalPortState(port);
    FAST_ZPIN_WRITE(currentPort->CS, true);
}

void Board_SPI_CS_Low(BoardSPIPort *port)
{
    Board_SPI_SetInternalPortState(port);
    FAST_ZPIN_WRITE(currentPort->CS, false);
}

void Board_SPI_Init(BoardSPIPort *port)
{
	__init_spi_handle(port);
    Board_SPI_SetInternalPortState(port);

    Board_Digital_Write(port->cs, HIGH);
    Board_Digital_Write(port->clk, HIGH);
    Board_Digital_Write(port->miso, HIGH);
    Board_Digital_Write(port->mosi, HIGH);

    Board_Digital_PinMode(port->cs, OUTPUT);
    Board_Digital_PinMode(port->clk, OUTPUT);
    Board_Digital_PinMode(port->miso, OUTPUT);
    Board_Digital_PinMode(port->mosi, OUTPUT);
}

void Board_SPI_End(BoardSPIPort *port)
{

}

#if 0
void Board_SPI_Init_hw(void)
{
	Chip_SSP_Init(LPC_SSP0);
	Chip_SSP_SetFormat(LPC_SSP0, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_BITS_8);
	Chip_SSP_SetMaster(LPC_SSP0, SSP_MODE_TEST);
	Chip_SSP_Enable(LPC_SSP0);

#if INTERRUPT_MODE
	/* Setting SSP interrupt */
	NVIC_EnableIRQ(SSP_IRQ);
#endif

#if POLLING_MODE
	Chip_SSP_RWFrames_Blocking(LPC_SSP, &xf_setup);
#elif INTERRUPT_MODE
	Chip_SSP_Int_FlushData(LPC_SSP);/* flush dummy data from SSP FiFO */
	if (SSP_DATA_BYTES(ssp_format.bits) == 1) {
		Chip_SSP_Int_RWFrames8Bits(LPC_SSP, &xf_setup);
	}
	else {
		Chip_SSP_Int_RWFrames16Bits(LPC_SSP, &xf_setup);
	}

	Chip_SSP_Int_Enable(LPC_SSP);			/* enable interrupt */
	while (!isXferCompleted) {}
#endif /*INTERRUPT_MODE*/
}
#endif
