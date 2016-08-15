/*
  board_wire.cpp - Arduino-XC HAL API implementation of Wire/I2C peripheral
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

#include "board/board_wire.h"
#include "ring_buffer.h"
#include "board/variant.h"
#include "lib/Wire.h"

#include "chip.h"
#include "i2cm_13xx.h"


WireClass Wire = WireClass((void*)LPC_I2C, NULL);

void WIRE_ISR_HANDLER(void)
{
    Wire.onService();
}

void Board_I2C_Master_Init(void *pI2C)
{
//    LPC_I2C_T *lpcI2C = (LPC_I2C_T *)pI2C;

//    Init_I2C_PinMux();

    /* Initialize I2C
     * - enable I2C clock via SYSAHBCLKCTRL
     * - resets I2C peripheral via clocking I2C pin in PRESETCTRL
     */
    Chip_I2CM_Init(LPC_I2C);
    /*
     * Sets SCLH and SCLL based on PCLK (which is same as core clock)
     * and given speed
     */
    Chip_I2CM_SetBusSpeed(LPC_I2C, SPEED_100KHZ);
    /*
     * Clears SI, STO, STA and AA bits
     * via CONCLR
     */
    Chip_I2CM_ResetControl(LPC_I2C);
    /*
     * clears I2EN bit via CONCLR
     * Chip_I2CM_SendStart() will set I2EN bit
     */
    Chip_I2CM_Disable(LPC_I2C);

    /*
     * Enable I2C interrupt for interrupt based I2C state handling
     */
//    NVIC_EnableIRQ(I2C0_IRQn);
}

void Board_I2C_Slave_Init(void *pI2C, uint8_t address)
{
//    LPC_I2C_T *lpcI2C = (LPC_I2C_T *)pI2C;
    // TODO
}

void Board_I2C_Set_Bus_Speed(void *pI2C, uint32_t frequency)
{
//    LPC_I2C_T *lpcI2C = (LPC_I2C_T *)pI2C;
    Chip_I2CM_SetBusSpeed(LPC_I2C, frequency);
}

/* Returns:
    0:success
    1:data too long to fit in transmit buffer
    2:received NACK on transmit of address
    3:received NACK on transmit of data
    4:other error
 */
uint32_t Board_I2C_Master_Read_Blocking(void *pI2C, uint8_t address, uint8_t *rxBuffer, uint16_t size)
{
    LPC_I2C_T *lpcI2C = (LPC_I2C_T *)pI2C;
    I2CM_XFER_T xfer;

    xfer.slaveAddr = address;
    xfer.rxSz = size;
    xfer.rxBuff = rxBuffer;
    xfer.txSz = 0;
    xfer.txBuff = NULL;

    // returns: xfer->status != I2CM_STATUS_BUSY;
    //    Chip_I2CM_XferBlocking(LPC_I2C, &xfer);
    Chip_I2CM_XferBlocking(lpcI2C, &xfer);
    Serial.println(xfer.status);

    return 0;
}

uint32_t Board_I2C_Master_Write_Blocking(void *pI2C, uint8_t address, uint8_t *txBuffer, uint16_t size)
{
    LPC_I2C_T *lpcI2C = (LPC_I2C_T *)pI2C;
    I2CM_XFER_T xfer;

    xfer.slaveAddr = address;
    xfer.rxSz = 0;
    xfer.rxBuff = NULL;
    xfer.txSz = size;
    xfer.txBuff = txBuffer;

    // returns: xfer->status != I2CM_STATUS_BUSY;
    Chip_I2CM_XferBlocking(lpcI2C, &xfer);

    return 0;
}
