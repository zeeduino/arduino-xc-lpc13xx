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

#include "chip.h"
#include "ssp_13xx.h"

//#define CS_BIT       APIN_PIN(SPI_BUILTIN_SSEL)
//#define CS_PORT      APIN_PORT(SPI_BUILTIN_SSEL)
//
//#define SCK_BIT      APIN_PIN(SPI_BUILTIN_SCK)
//#define SCK_PORT     APIN_PORT(SPI_BUILTIN_SCK)
//
//#define MISO_BIT     APIN_PIN(SPI_BUILTIN_MISO)
//#define MISO_PORT    APIN_PORT(SPI_BUILTIN_MISO)
//
//#define MOSI_BIT     APIN_PIN(SPI_BUILTIN_MOSI)
//#define MOSI_PORT    APIN_PORT(SPI_BUILTIN_MOSI)

//#define BB_DELAY() delayMicroseconds(5)
#define BB_DELAY()

//#define CS_LOW() Chip_GPIO_WritePortBit(LPC_GPIO_PORT, CS_PORT, CS_BIT, false)
//#define CS_HIGH() Chip_GPIO_WritePortBit(LPC_GPIO_PORT, CS_PORT, CS_BIT, true)
//#define SCK_LOW() Chip_GPIO_WritePortBit(LPC_GPIO_PORT, SCK_PORT, SCK_BIT, false)
//#define SCK_HIGH() Chip_GPIO_WritePortBit(LPC_GPIO_PORT, SCK_PORT, SCK_BIT, true)
//#define MISO() Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, MISO_PORT, MISO_BIT)
//#define MOSI(x) Chip_GPIO_WritePortBit(LPC_GPIO_PORT, MOSI_PORT, MOSI_BIT, x)

#define CS_LOW() Chip_GPIO_WritePortBit(LPC_GPIO_PORT, CS_PORTv, CS_BITv, false)
#define CS_HIGH() Chip_GPIO_WritePortBit(LPC_GPIO_PORT, CS_PORTv, CS_BITv, true)
#define SCK_LOW() Chip_GPIO_WritePortBit(LPC_GPIO_PORT, SCK_PORTv, SCK_BITv, false)
#define SCK_HIGH() Chip_GPIO_WritePortBit(LPC_GPIO_PORT, SCK_PORTv, SCK_BITv, true)
#define MISO() Chip_GPIO_ReadPortBit(LPC_GPIO_PORT, MISO_PORTv, MISO_BITv)
#define MOSI(x) Chip_GPIO_WritePortBit(LPC_GPIO_PORT, MOSI_PORTv, MOSI_BITv, x)

uint8_t CS_BITv;
uint8_t CS_PORTv;
uint8_t SCK_BITv;
uint8_t SCK_PORTv;
uint8_t MISO_BITv;
uint8_t MISO_PORTv;
uint8_t MOSI_BITv;
uint8_t MOSI_PORTv;

void Board_SPI_SetInternalPortStateDefault(BoardSPIPort *port)
{
	CS_BITv    =   APIN_PIN(SPI_BUILTIN_SSEL);
	CS_PORTv   =   APIN_PORT(SPI_BUILTIN_SSEL);

	SCK_BITv   =   APIN_PIN(SPI_BUILTIN_SCK);
	SCK_PORTv  =   APIN_PORT(SPI_BUILTIN_SCK);

	MISO_BITv  =   APIN_PIN(SPI_BUILTIN_MISO);
	MISO_PORTv =   APIN_PORT(SPI_BUILTIN_MISO);

	MOSI_BITv  =    APIN_PIN(SPI_BUILTIN_MOSI);
	MOSI_PORTv =    APIN_PORT(SPI_BUILTIN_MOSI);
}

void Board_SPI_SetInternalPortState(BoardSPIPort *port)
{
	CS_BITv    =   APIN_PIN(port->cs);
	CS_PORTv   =   APIN_PORT(port->cs);

	SCK_BITv   =   APIN_PIN(port->clk);
	SCK_PORTv  =   APIN_PORT(port->clk);

	MISO_BITv  =   APIN_PIN(port->miso);
	MISO_PORTv =   APIN_PORT(port->miso);

	MOSI_BITv  =    APIN_PIN(port->mosi);
	MOSI_PORTv =    APIN_PORT(port->mosi);
}

uint8_t Board_SPI_Transfer(BoardSPIPort *port, uint8_t data)
{
    uint8_t rx;
    int i;

    Board_SPI_SetInternalPortState(port);

//    CS_LOW();
    BB_DELAY();
    rx = 0x00;
    for (i = 7; i >= 0; i--)
    {
        rx <<= 1;

        SCK_LOW();
        BB_DELAY();

        MOSI(data & (1 << i));
        BB_DELAY();

        SCK_HIGH();
        BB_DELAY();
        if (MISO())
            rx |= 1;
        BB_DELAY();
    }
//    MOSI(true); // MOSI HIGH when idle

    return rx;
}

void Board_SPI_CS_High(BoardSPIPort *port)
{
    Board_SPI_SetInternalPortState(port);
    Chip_GPIO_WritePortBit(LPC_GPIO_PORT, 1, CS_BITv, true);
}

void Board_SPI_CS_Low(BoardSPIPort *port)
{
    Board_SPI_SetInternalPortState(port);
    Chip_GPIO_WritePortBit(LPC_GPIO_PORT, 1, CS_BITv, false);
}

void Board_SPI_Init(BoardSPIPort *port)
{
    Board_SPI_SetInternalPortState(port);

    Board_Digital_Write(port->cs, HIGH);
    Board_Digital_Write(port->clk, HIGH);
    Board_Digital_Write(port->miso, HIGH);
    Board_Digital_Write(port->mosi, HIGH);

//    Chip_GPIO_WritePortBit(LPC_GPIO_PORT, CS_PORTv, CS_BITv, true);
//    Chip_GPIO_WritePortBit(LPC_GPIO_PORT, SCK_PORTv, SCK_BITv, true);
//    Chip_GPIO_WritePortBit(LPC_GPIO_PORT, MISO_PORTv, MISO_BITv, true);
//    Chip_GPIO_WritePortBit(LPC_GPIO_PORT, MOSI_PORTv, MOSI_BITv, true);

    Board_Digital_PinMode(port->cs, OUTPUT);
    Board_Digital_PinMode(port->clk, OUTPUT);
    Board_Digital_PinMode(port->miso, OUTPUT);
    Board_Digital_PinMode(port->mosi, OUTPUT);

//    Chip_IOCON_PinMuxSet(LPC_IOCON, CS_PORTv, CS_BITv, (IOCON_FUNC0 | IOCON_MODE_PULLUP)); /* PIO1_19 connected to SSEL1 */
//    Chip_GPIO_WriteDirBit(LPC_GPIO_PORT, CS_PORTv, CS_BITv, true); /* output */
//    Chip_IOCON_PinMuxSet(LPC_IOCON, SCK_PORTv, SCK_BITv, (IOCON_FUNC0 | IOCON_MODE_PULLUP)); /* PIO1_20 connected to SCK1 */
//    Chip_GPIO_WriteDirBit(LPC_GPIO_PORT, SCK_PORTv, SCK_BITv, true); /* output */
//    Chip_IOCON_PinMuxSet(LPC_IOCON, MISO_PORTv, MISO_BITv, (IOCON_FUNC0 | IOCON_MODE_PULLUP)); /* PIO1_21 connected to MISO1 */
//    Chip_GPIO_WriteDirBit(LPC_GPIO_PORT, MISO_PORTv, MISO_BITv, false); /* input */
    // This will give us MOSI HIGH when idle
//    Chip_IOCON_PinMuxSet(LPC_IOCON, MOSI_PORTv, MOSI_BITv, (IOCON_FUNC0 | IOCON_MODE_PULLUP)); /* PIO1_22 connected to MOSI1 */
    // This will give us MOSI LOW when idle
    //Chip_IOCON_PinMuxSet(LPC_IOCON, MOSI_PORTv, MOSI_BITv, (IOCON_FUNC0 | IOCON_MODE_PULLDOWN));
//    Chip_GPIO_WriteDirBit(LPC_GPIO_PORT, MOSI_PORTv, MOSI_BITv, true); /* output */
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
