//*****************************************************************************
//
// i2c_atmel.c - I2C master example.
//
// Copyright (c) 2005-2007 Luminary Micro, Inc.  All rights reserved.
// 
// Software License Agreement
// 
// Luminary Micro, Inc. (LMI) is supplying this software for use solely and
// exclusively on LMI's microcontroller products.
// 
// The software is owned by LMI and/or its suppliers, and is protected under
// applicable copyright laws.  All rights are reserved.  You may not combine
// this software with "viral" open-source software in order to form a larger
// program.  Any use in violation of the foregoing restrictions may subject
// the user to criminal sanctions under applicable laws, as well as to civil
// liability for the breach of the terms and conditions of this license.
// 
// THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
// CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 1900 of the Stellaris Peripheral Driver Library.
//
//*****************************************************************************

#include "../../../hw_ints.h"
#include "../../../hw_memmap.h"
#include "../../../hw_types.h"
#include "../../../src/debug.h"
#include "../../../src/gpio.h"
#include "../../../src/i2c.h"
#include "../../../src/interrupt.h"
#include "../../../src/sysctl.h"
#include "../../../utils/diag.h"
#include "../pdc.h"

//*****************************************************************************
//
//! \addtogroup dk_lm3s102_list
//! <h1>I2C (i2c_atmel)</h1>
//!
//! This example application uses the I2C master to communicate with the Atmel
//! AT24C08A EEPROM that is on the development board.  The first sixteen bytes
//! of the EEPROM are erased and then programmed with an incrementing sequence.
//! The data is then read back to verify its correctness.  The transfer is
//! managed by an interrupt handler in response to the I2C interrupt; since a
//! sixteen-byte read at a 100 kHz I2C bus speed takes almost 2 ms, this allows
//! a lot of other processing to occur during the transfer (though that time is
//! not utilized by this example).
//!
//! In order for this example to work properly, the I2C_SCL (JP14), I2C_SDA
//! (JP13), and I2CM_A2 (JP11) jumpers must be installed on the board, and the
//! I2CM_WP (JP12) jumper must be removed.
//
//*****************************************************************************

//*****************************************************************************
//
// The states in the interrupt handler state machine.
//
//*****************************************************************************
#define STATE_IDLE         0
#define STATE_WRITE_NEXT   1
#define STATE_WRITE_FINAL  2
#define STATE_WAIT_ACK     3
#define STATE_SEND_ACK     4
#define STATE_READ_ONE     5
#define STATE_READ_FIRST   6
#define STATE_READ_NEXT    7
#define STATE_READ_FINAL   8
#define STATE_READ_WAIT    9

//*****************************************************************************
//
// The variables that track the data to be transmitted or received.
//
//*****************************************************************************
static unsigned char *g_pucData = 0;
static unsigned long g_ulCount = 0;

//*****************************************************************************
//
// The current state of the interrupt handler state machine.
//
//*****************************************************************************
static volatile unsigned long g_ulState = STATE_IDLE;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

//*****************************************************************************
//
// The I2C interrupt handler.
//
//*****************************************************************************
void
I2CIntHandler(void)
{
    //
    // Clear the I2C interrupt.
    //
    I2CMasterIntClear(I2C0_MASTER_BASE);

    //
    // Determine what to do based on the current state.
    //
    switch(g_ulState)
    {
        //
        // The idle state.
        //
        case STATE_IDLE:
        {
            //
            // There is nothing to be done.
            //
            break;
        }

        //
        // The state for the middle of a burst write.
        //
        case STATE_WRITE_NEXT:
        {
            //
            // Write the next byte to the data register.
            //
            I2CMasterDataPut(I2C0_MASTER_BASE, *g_pucData++);
            g_ulCount--;

            //
            // Continue the burst write.
            //
            I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

            //
            // If there is one byte left, set the next state to the final write
            // state.
            //
            if(g_ulCount == 1)
            {
                g_ulState = STATE_WRITE_FINAL;
            }

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the final write of a burst sequence.
        //
        case STATE_WRITE_FINAL:
        {
            //
            // Write the final byte to the data register.
            //
            I2CMasterDataPut(I2C0_MASTER_BASE, *g_pucData++);
            g_ulCount--;

            //
            // Finish the burst write.
            //
            I2CMasterControl(I2C0_MASTER_BASE,
                             I2C_MASTER_CMD_BURST_SEND_FINISH);

            //
            // The next state is to wait for the burst write to complete.
            //
            g_ulState = STATE_SEND_ACK;

            //
            // This state is done.
            //
            break;
        }

        //
        // Wait for an ACK on the read after a write.
        //
        case STATE_WAIT_ACK:
        {
            //
            // See if there was an error on the previously issued read.
            //
            if(I2CMasterErr(I2C0_MASTER_BASE) == I2C_MASTER_ERR_NONE)
            {
                //
                // Read the byte received.
                //
                I2CMasterDataGet(I2C0_MASTER_BASE);

                //
                // There was no error, so the state machine is now idle.
                //
                g_ulState = STATE_IDLE;

                //
                // This state is done.
                //
                break;
            }

            //
            // Fall through to STATE_SEND_ACK.
            //
        }

        //
        // Send a read request, looking for the ACK to indicate that the write
        // is done.
        //
        case STATE_SEND_ACK:
        {
            //
            // Put the I2C master into receive mode.
            //
            I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x50, true);

            //
            // Perform a single byte read.
            //
            I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

            //
            // The next state is the wait for the ack.
            //
            g_ulState = STATE_WAIT_ACK;

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for a single byte read.
        //
        case STATE_READ_ONE:
        {
            //
            // Put the I2C master into receive mode.
            //
            I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x50, true);

            //
            // Perform a single byte read.
            //
            I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

            //
            // The next state is the wait for final read state.
            //
            g_ulState = STATE_READ_WAIT;

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the start of a burst read.
        //
        case STATE_READ_FIRST:
        {
            //
            // Put the I2C master into receive mode.
            //
            I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x50, true);

            //
            // Start the burst receive.
            //
            I2CMasterControl(I2C0_MASTER_BASE,
                             I2C_MASTER_CMD_BURST_RECEIVE_START);

            //
            // The next state is the middle of the burst read.
            //
            g_ulState = STATE_READ_NEXT;

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the middle of a burst read.
        //
        case STATE_READ_NEXT:
        {
            //
            // Read the received character.
            //
            *g_pucData++ = I2CMasterDataGet(I2C0_MASTER_BASE);
            g_ulCount--;

            //
            // Continue the burst read.
            //
            I2CMasterControl(I2C0_MASTER_BASE,
                             I2C_MASTER_CMD_BURST_RECEIVE_CONT);

            //
            // If there are two characters left to be read, make the next
            // state be the end of burst read state.
            //
            if(g_ulCount == 2)
            {
                g_ulState = STATE_READ_FINAL;
            }

            //
            // This state is done.
            //
            break;
        }

        //
        // The state for the end of a burst read.
        //
        case STATE_READ_FINAL:
        {
            //
            // Read the received character.
            //
            *g_pucData++ = I2CMasterDataGet(I2C0_MASTER_BASE);
            g_ulCount--;

            //
            // Finish the burst read.
            //
            I2CMasterControl(I2C0_MASTER_BASE,
                             I2C_MASTER_CMD_BURST_RECEIVE_FINISH);

            //
            // The next state is the wait for final read state.
            //
            g_ulState = STATE_READ_WAIT;

            //
            // This state is done.
            //
            break;
        }

        //
        // This state is for the final read of a single or burst read.
        //
        case STATE_READ_WAIT:
        {
            //
            // Read the received character.
            //
            *g_pucData++  = I2CMasterDataGet(I2C0_MASTER_BASE);
            g_ulCount--;

            //
            // The state machine is now idle.
            //
            g_ulState = STATE_IDLE;

            //
            // This state is done.
            //
            break;
        }
    }
}

//*****************************************************************************
//
// Write to the Atmel device.
//
//*****************************************************************************
void
AtmelWrite(unsigned char *pucData, unsigned long ulOffset,
           unsigned long ulCount)
{
    //
    // Save the data buffer to be written.
    //
    g_pucData = pucData;
    g_ulCount = ulCount;

    //
    // Set the next state of the interrupt state machine based on the number of
    // bytes to write.
    //
    if(ulCount != 1)
    {
        g_ulState = STATE_WRITE_NEXT;
    }
    else
    {
        g_ulState = STATE_WRITE_FINAL;
    }

    //
    // Set the slave address and setup for a transmit operation.
    //
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x50 | (ulOffset >> 8), false);

    //
    // Place the address to be written in the data register.
    //
    I2CMasterDataPut(I2C0_MASTER_BASE, ulOffset);

    //
    // Start the burst cycle, writing the address as the first byte.
    //
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //
    // Wait until the I2C interrupt state machine is idle.
    //
    while(g_ulState != STATE_IDLE)
    {
    }
}

//*****************************************************************************
//
// Read from the Atmel device.
//
//*****************************************************************************
void
AtmelRead(unsigned char *pucData, unsigned long ulOffset,
          unsigned long ulCount)
{
    //
    // Save the data buffer to be read.
    //
    g_pucData = pucData;
    g_ulCount = ulCount;

    //
    // Set the next state of the interrupt state machine based on the number of
    // bytes to read.
    //
    if(ulCount == 1)
    {
        g_ulState = STATE_READ_ONE;
    }
    else
    {
        g_ulState = STATE_READ_FIRST;
    }

    //
    // Start with a dummy write to get the address set in the EEPROM.
    //
    I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x50 | (ulOffset >> 8), false);

    //
    // Place the address to be written in the data register.
    //
    I2CMasterDataPut(I2C0_MASTER_BASE, ulOffset);

    //
    // Perform a single send, writing the address as the only byte.
    //
    I2CMasterControl(I2C0_MASTER_BASE, I2C_MASTER_CMD_SINGLE_SEND);

    //
    // Wait until the I2C interrupt state machine is idle.
    //
    while(g_ulState != STATE_IDLE)
    {
    }
}

//*****************************************************************************
//
// This example demonstrates the use of the I2C block to connect to an Atmel
// AT24C08A EEPROM.
//
//*****************************************************************************
int
main(void)
{
    unsigned char pucData[16];
    unsigned long ulIdx;

    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_6MHZ);

    //
    // Init the PDC and the LCD.
    //
    PDCInit();
    PDCLCDInit();
    PDCLCDBacklightOn();

    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //
    // Indicate that the I2C example is running.
    //
    PDCLCDSetPos(0, 0);
    PDCLCDWrite("I2C running...", 14);

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Configure the appropriate pins to be I2C instead of GPIO.
    //
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3);

    //
    // Initialize the I2C master.
    //
    I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), false);

    //
    // Enable the I2C interrupt.
    //
    IntEnable(INT_I2C0);

    //
    // Enable the I2C master interrupt.
    //
    I2CMasterIntEnable(I2C0_MASTER_BASE);

    //
    // Write a data=address pattern into the first 16 bytes of the Atmel
    // device.
    //
    for(ulIdx = 0; ulIdx < 16; ulIdx++)
    {
        pucData[ulIdx] = ulIdx;
    }
    AtmelWrite(pucData, 0, 16);

    //
    // Read back the first 16 bytes of the Atmel device and verify that it
    // contains the data it should.
    //
    AtmelRead(pucData, 0, 16);
    for(ulIdx = 0; ulIdx < 16; ulIdx++)
    {
        if(pucData[ulIdx] != ulIdx)
        {
            PDCLCDSetPos(0, 1);
            PDCLCDWrite("Data error.", 11);
            DiagExit(0);
        }
    }

    //
    // Success.
    //
    PDCLCDSetPos(0, 1);
    PDCLCDWrite("Success.", 8);

    //
    // Exit.
    //
    DiagExit(0);
}
