//*****************************************************************************
//
// ssi_atmel.c - SSI example.
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
#include "../../../src/interrupt.h"
#include "../../../src/ssi.h"
#include "../../../src/sysctl.h"
#include "../../../utils/diag.h"
#include "../pdc.h"

//*****************************************************************************
//
//! \addtogroup dk_lm3s811_list
//! <h1>SSI (ssi_atmel)</h1>
//!
//! This example application uses the SSI master to communicate with the Atmel
//! AT25F1024A EEPROM that is on the development board.  The first 256 bytes of
//! the EEPROM are erased and then programmed with an incrementing sequence.
//! The data is then read back to verify its correctness.  The transfer is
//! managed by an interrupt handler in response to the SSI interrupt; since a
//! 256-byte read at a 1 MHz SSI bus speed takes around 2 ms, this allows a lot
//! of other processing to occur during the transfer (though that time is not
//! utilized by this example).
//
//*****************************************************************************

//*****************************************************************************
//
// The GPIO port A pin numbers for the various SSI signals.
//
//*****************************************************************************
#define SSI_CS                  GPIO_PIN_3
#define SSI_CLK                 GPIO_PIN_2
#define SSI_TX                  GPIO_PIN_5
#define SSI_RX                  GPIO_PIN_4

//*****************************************************************************
//
// Commands that can be sent to the Atmel AT25F1024A.
//
//*****************************************************************************
#define ATMEL_WREN              0x06        // Set write enable latch
#define ATMEL_WRDI              0x04        // Reset write enable latch
#define ATMEL_RDSR              0x05        // Read status register
#define ATMEL_WRSR              0x01        // Write status register
#define ATMEL_READ              0x03        // Read data from memory array
#define ATMEL_PROGRAM           0x02        // Program data into memory array
#define ATMEL_SECTOR_ERASE      0x52        // Erase one sector in memory array
#define ATMEL_CHIP_ERASE        0x62        // Erase all sectors in memory arr.
#define ATMEL_RDID              0x15        // Read manufacturer and product ID

//*****************************************************************************
//
// A buffer to contain data written to or read from the Atmel device.
//
//*****************************************************************************
static unsigned char g_pucData[260];

//*****************************************************************************
//
// The variables that track the data to be transmitted.
//
//*****************************************************************************
static unsigned char *g_pucDataOut;
static unsigned long g_ulOutCount;
static unsigned long g_ulOutExtra;

//*****************************************************************************
//
// The variables that track the data to be received.
//
//*****************************************************************************
static unsigned char *g_pucDataIn;
static unsigned long g_ulInCount;
static unsigned long g_ulInExtra;

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
// The interrupt handler for the SSI interrupt.
//
//*****************************************************************************
void
SSIIntHandler(void)
{
    unsigned long ulStatus, ulCount, ulData;

    //
    // Get the reason for the interrupt.
    //
    ulStatus = SSIIntStatus(SSI0_BASE, true);

    //
    // See if the receive interrupt is being asserted.
    //
    if(ulStatus & (SSI_RXFF | SSI_RXTO))
    {
        //
        // Loop as many times as required to empty the FIFO.
        //
        while(1)
        {
            //
            // Read a byte from the FIFO if possible.  Break out of the loop if
            // the FIFO is empty.
            //
            if(SSIDataGetNonBlocking(SSI0_BASE, &ulData) == 0)
            {
                break;
            }

            //
            // See if this byte needs to be saved.
            //
            if(g_ulInCount)
            {
                //
                // Save this byte.
                //
                *g_pucDataIn++ = ulData;

                //
                // Decrement the count of bytes to save.
                //
                g_ulInCount--;
            }
            else
            {
                //
                // Decrement the count of extra bytes to read.
                //
                g_ulInExtra--;
            }
        }

        //
        // See if all data has been transmitted and received.
        //
        if((g_ulInCount + g_ulInExtra + g_ulOutCount + g_ulOutExtra) == 0)
        {
            //
            // All data has been transmitted and received, so disable the
            // receive interrupt.
            //
            SSIIntDisable(SSI0_BASE, SSI_RXFF | SSI_RXTO);

            //
            // Deassert the SSI chip select.
            //
            GPIOPinWrite(GPIO_PORTA_BASE, SSI_CS, SSI_CS);
        }
    }

    //
    // See if the transmit interrupt is being asserted.
    //
    if(ulStatus & SSI_TXFF)
    {
        //
        // Write up to four bytes into the FIFO.
        //
        for(ulCount = 0; ulCount < 4; ulCount++)
        {
            //
            // See if there is more data to be transmitted.
            //
            if(g_ulOutCount)
            {
                //
                // Transmit this byte if possible.  Break out of the loop if
                // the FIFO is full.
                //
                if(SSIDataPutNonBlocking(SSI0_BASE, *g_pucDataOut) == 0)
                {
                    break;
                }

                //
                // Decrement the count of bytes to be transmitted.
                //
                g_pucDataOut++;
                g_ulOutCount--;
            }

            //
            // See if there are more extra bytes to be transmitted.
            //
            else if(g_ulOutExtra)
            {
                //
                // Transmit this extra byte if possible.  Break out of the loop
                // if the FIFO is full.
                //
                if(SSIDataPutNonBlocking(SSI0_BASE, 0) == 0)
                {
                    break;
                }

                //
                // Decrement the count of extra bytes to be transmitted.
                //
                g_ulOutExtra--;
            }

            //
            // Otherwise, stop transmitting data.
            //
            else
            {
                //
                // Disable the transmit interrupt since all data to be
                // transmitted has been written into the FIFO.
                //
                SSIIntDisable(SSI0_BASE, SSI_TXFF);

                //
                // Break out of the loop since there is no more data to
                // transmit.
                //
                break;
            }
        }
    }
}

//*****************************************************************************
//
// This will start an interrupt driven transfer to the SSI peripheral.
//
//*****************************************************************************
void
SSITransfer(unsigned char *pucDataOut, unsigned long ulOutCount,
            unsigned char *pucDataIn, unsigned long ulInCount)
{
    //
    // Save the output data buffer.
    //
    g_pucDataOut = pucDataOut;
    g_ulOutCount = ulOutCount;

    //
    // Save the input data buffer.
    //
    g_pucDataIn = pucDataIn;
    g_ulInCount = ulInCount;

    //
    // Compute the number of extra bytes to send or receive.  These counts make
    // the number of bytes transmitted equal to the number of bytes received;
    // a requirement of the SSI peripheral.
    //
    if(ulInCount > ulOutCount)
    {
        g_ulOutExtra = ulInCount - ulOutCount;
        g_ulInExtra = 0;
    }
    else
    {
        g_ulOutExtra = 0;
        g_ulInExtra = ulOutCount - ulInCount;
    }

    //
    // Assert the SSI chip select.
    //
    GPIOPinWrite(GPIO_PORTA_BASE, SSI_CS, 0);

    //
    // Enable the transmit and receive interrupts.  This will start the actual
    // transfer.
    //
    SSIIntEnable(SSI0_BASE, SSI_TXFF | SSI_RXFF | SSI_RXTO);

    //
    // Wait until the SSI chip select deasserts, indicating the end of the
    // transfer.
    //
    while(!(GPIOPinRead(GPIO_PORTA_BASE, SSI_CS) & SSI_CS))
    {
    }
}

//*****************************************************************************
//
// Read the manufacturer and produce ID from the Atmel device.
//
//*****************************************************************************
unsigned long
AtmelReadID(void)
{
    unsigned char pucCmd[4];

    //
    // Send the read ID command.
    //
    pucCmd[0] = ATMEL_RDID;

    //
    // Do the transfer.  There is one byte to write but three to read (the byte
    // that gets transferred during the write and two bytes of ID).
    //
    SSITransfer(pucCmd, 1, pucCmd, 3);

    //
    // Return the ID.
    //
    return((pucCmd[1] << 8) | pucCmd[2]);
}

//*****************************************************************************
//
// Read the chip status from the Atmel device.
//
//*****************************************************************************
unsigned long
AtmelChipStatus(void)
{
    unsigned char pucCmd[4];

    //
    // Send the read status register command.
    //
    pucCmd[0] = ATMEL_RDSR;

    //
    // Do the transfer.  There is one byte to write but two to read (the byte
    // that gets transferred during the write and one status byte).
    //
    SSITransfer(pucCmd, 1, pucCmd, 2);

    //
    // Return the status byte.
    //
    return(pucCmd[1]);
}

//*****************************************************************************
//
// Erase the Atmel device.
//
//*****************************************************************************
void
AtmelEraseChip(void)
{
    unsigned char pucCmd[4];

    //
    // Enable the write latch.
    //
    pucCmd[0] = ATMEL_WREN;

    //
    // Send the command.
    //
    SSITransfer(pucCmd, 1, pucCmd, 1);

    //
    // Send the chip erase command.
    //
    pucCmd[0] = ATMEL_CHIP_ERASE;

    //
    // Send the command.
    //
    SSITransfer(pucCmd, 1, 0, 0);

    //
    // Wait until the erase is done.
    //
    while(AtmelChipStatus() & 0x01)
    {
    }
}

//*****************************************************************************
//
// Write to the Atmel device.  Note that pucData MUST be preceeded by four
// bytes that can be destroyed by this function.
//
//*****************************************************************************
void
AtmelWrite(unsigned char *pucData, unsigned long ulOffset,
           unsigned long ulCount)
{
    unsigned char pucCmd[4];

    //
    // Enable the write latch.
    //
    pucCmd[0] = ATMEL_WREN;

    //
    // Send the command.
    //
    SSITransfer(pucCmd, 1, pucCmd, 1);

    //
    // Send the program command.
    //
    pucData[-4] = ATMEL_PROGRAM;

    //
    // Send the address.
    //
    pucData[-3] = (ulOffset >> 16) & 0xff;
    pucData[-2] = (ulOffset >> 8) & 0xff;
    pucData[-1] = ulOffset & 0xff;

    //
    // Send the command.
    //
    SSITransfer(pucData - 4, ulCount + 4, 0, 0);

    //
    // Wait until the programming is done.
    //
    while(AtmelChipStatus() & 0x01)
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
    unsigned char pucCmd[4];

    //
    // Send the read command.
    //
    pucCmd[0] = ATMEL_READ;

    //
    // Send the address.
    //
    pucCmd[1] = (ulOffset >> 16) & 0xff;
    pucCmd[2] = (ulOffset >> 8) & 0xff;
    pucCmd[3] = ulOffset & 0xff;

    //
    // Send the command.
    //
    SSITransfer(pucCmd, 4, pucData - 4, ulCount + 4);
}

//*****************************************************************************
//
// This example demonstrates the use of the SSI block to connect to an Atmel
// AT25F1024A EEPROM.
//
//*****************************************************************************
int
main(void)
{
    unsigned char *pucData;
    unsigned long ulIdx;

    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_6MHZ);

    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Init the PDC and the LCD.
    //
    PDCInit();
    PDCLCDInit();
    PDCLCDBacklightOn();
    PDCLCDSetPos(0, 0);
    PDCLCDWrite("SSI running...", 14);

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Configure the appropriate pins to be SSI instead of GPIO.  Note that
    // the chip select is kept as a GPIO to guarantee the appropriate
    // signalling to the Atmel device.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, SSI_CS);
    GPIOPinWrite(GPIO_PORTA_BASE, SSI_CS, SSI_CS);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, SSI_CLK | SSI_TX | SSI_RX);

    //
    // Configure and enable the SSI port for master mode.
    //
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_3,
                       SSI_MODE_MASTER, 1000000, 8);
    SSIEnable(SSI0_BASE);

    //
    // Read any residual data from the SSI port.
    //
    while(SSIDataGetNonBlocking(SSI0_BASE, &ulIdx))
    {
    }

    //
    // Enable the SSI interrupt.
    //
    IntEnable(INT_SSI0);

    //
    // Make sure that the Atmel device ID is correct.
    //
    if(AtmelReadID() != 0x1f60)
    {
        //
        // Must reinitialize the PDC because the SSI setup for the
        // flash device is different from the PDC.
        //
        PDCInit();
        PDCLCDSetPos(0, 1);
        PDCLCDWrite("Bad device ID.", 14);
        DiagExit(0);
    }

    //
    // Erase the Atmel device.
    //
    AtmelEraseChip();

    //
    // For simplified operation, the AtmelWrite() and AtmelRead() functions
    // requires four bytes of padding BEFORE the buffer passed to them (used to
    // store the command to be sent to the EEPROM before the data).  Construct
    // a pointer four bytes into the data buffer.
    //
    pucData = g_pucData + 4;

    //
    // Write a data=address pattern into the first 256 bytes of the Atmel
    // device.
    //
    for(ulIdx = 0; ulIdx < 256; ulIdx++)
    {
        pucData[ulIdx] = ulIdx;
    }
    AtmelWrite(pucData, 0, 256);

    //
    // Read the first 256 bytes of the Atmel device and verify that it contains
    // the data it should.
    //
    AtmelRead(pucData, 0, 256);
    for(ulIdx = 0; ulIdx < 256; ulIdx++)
    {
        if(pucData[ulIdx] != ulIdx)
        {
            //
            // Must reinitialize the PDC because the SSI setup for the
            // flash device is different from the PDC.
            //
            PDCInit();
            PDCLCDSetPos(0, 1);
            PDCLCDWrite("Read error.", 11);
            DiagExit(0);
        }
    }

    //
    // Success.  Must reinitialize the PDC because the SSI setup for the
    // the flash device is different from the PDC.
    //
    PDCInit();
    PDCLCDSetPos(0, 1);
    PDCLCDWrite("Success.", 8);

    //
    // Exit.
    //
    DiagExit(0);
}
