//*****************************************************************************
//
// bl_ssi.c - Functions used to transfer data via the SSI port.
//
// Copyright (c) 2006-2007 Luminary Micro, Inc.  All rights reserved.
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

#include "../hw_gpio.h"
#include "../hw_memmap.h"
#include "../hw_ssi.h"
#include "../hw_sysctl.h"
#include "../hw_types.h"
#include "bl_config.h"
#include "bl_ssi.h"

//*****************************************************************************
//
//! \addtogroup boot_loader_api
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
//! Sends data via the SSI port in slave mode.
//!
//! \param pucData is the location of the data to send through the SSI port.
//! \param ulSize is the number of bytes of data to send.
//!
//! This function sends data through the SSI port in slave mode.  This function
//! will not return until all bytes are sent.
//!
//! This function is contained in <tt>bl_ssi.c</tt>.
//!
//! \return None.
//
//*****************************************************************************
void
SSISend(const unsigned char *pucData, unsigned long ulSize)
{
    //
    // Send the requested number of bytes over the SSI port.
    //
    while(ulSize--)
    {
        //
        // Wait until there is space in the SSI FIFO.
        //
        while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TNF))
        {
        }

        //
        // Write the next byte to the SSI port.
        //
        HWREG(SSI0_BASE + SSI_O_DR) = *pucData++;
    }

    //
    // Empty the receive FIFO.
    //
    while(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE)
    {
        HWREG(SSI0_BASE + SSI_O_DR);
    }
}

//*****************************************************************************
//
//! Waits until all data has been transmitted by the SSI port.
//!
//! This function waits until all data written to the SSI port has been read by
//! the master.
//!
//! This function is contained in <tt>bl_ssi.c</tt>.
//!
//! \return None.
//
//*****************************************************************************
void
SSIFlush(void)
{
    //
    // Wait until the transmit FIFO is empty.
    //
    while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_TFE))
    {
    }

    //
    // Wait until the interface is not busy.
    //
    while(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_BSY)
    {
    }
}

//*****************************************************************************
//
//! Receives data from the SSI port in slave mode.
//!
//! \param pucData is the location to store the data received from the SSI
//! port.
//! \param ulSize is the number of bytes of data to receive.
//!
//! This function receives data from the SSI port in slave mode.  The function
//! will not return until \e ulSize number of bytes have been received.
//!
//! This function is contained in <tt>bl_ssi.c</tt>.
//!
//! \return None.
//
//*****************************************************************************
void
SSIReceive(unsigned char *pucData, unsigned long ulSize)
{
    //
    // Ensure that we are sending out zeros so that we don't confuse the host.
    //
    HWREG(SSI0_BASE + SSI_O_DR) = 0;

    //
    // Wait for the requested number of bytes.
    //
    while(ulSize--)
    {
        //
        // Wait until there is data in the FIFO.
        //
        while(!(HWREG(SSI0_BASE + SSI_O_SR) & SSI_SR_RNE))
        {
        }

        //
        // Read the next byte from the FIFO.
        //
        *pucData++ = HWREG(SSI0_BASE + SSI_O_DR);
        HWREG(SSI0_BASE + SSI_O_DR) = 0;
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
