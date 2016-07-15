/*
  board_spi.cpp - Arduino-XC HAL API implementation of SPI peripheral
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
#include "board/board_spi.h"
#include "pins_arduino.h"

#include "chip.h"
#include "ssp_13xx.h"

static BoardSPIPort SPIPort = {
        (void*)LPC_SSP0,
        SPI_BUILTIN_SSEL,
        SPI_BUILTIN_SCK,
        SPI_BUILTIN_MISO,
        SPI_BUILTIN_MOSI,
        0
};
SPIClass SPI(&SPIPort, 0, NULL);
//SPIClass SPI(LPC_SSP0, 0, NULL);
