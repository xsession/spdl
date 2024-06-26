//*****************************************************************************
//
// bl_ssi.h - Definitions for the SSI transport functions.
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

#ifndef __BL_SSI_H__
#define __BL_SSI_H__

//*****************************************************************************
//
// This is the number of bits per transfer for SSI.  This is a constant and
// cannot be changed without corresponding code changes.
//
//*****************************************************************************
#define DATA_BITS_SSI           8

//*****************************************************************************
//
// This defines the SSI chip select pin that is being used by the boot loader.
//
//*****************************************************************************
#define SSI_CS                  (1 << 3)

//*****************************************************************************
//
// This defines the SSI clock pin that is being used by the boot loader.
//
//*****************************************************************************
#define SSI_CLK                 (1 << 2)

//*****************************************************************************
//
// This defines the SSI transmit pin that is being used by the boot loader.
//
//*****************************************************************************
#define SSI_TX                  (1 << 5)

//*****************************************************************************
//
// This defines the SSI receive pin that is being used by the boot loader.
//
//*****************************************************************************
#define SSI_RX                  (1 << 4)

//*****************************************************************************
//
// This defines the combination of pins used to implement the SSI port used by
// the boot loader.
//
//*****************************************************************************
#define SSI_PINS                (SSI_CLK | SSI_TX | SSI_RX | SSI_CS)

//*****************************************************************************
//
// SSI Transport APIs
//
//*****************************************************************************
extern void SSISend(unsigned char const *pucData, unsigned long ulSize);
extern void SSIReceive(unsigned char *pucData, unsigned long ulSize);
extern void SSIFlush(void);

//*****************************************************************************
//
// Define the transport functions if the SSI port is being used.
//
//*****************************************************************************
#ifdef SSI_ENABLE_UPDATE
#define SendData                SSISend
#define FlushData               SSIFlush
#define ReceiveData             SSIReceive
#endif

#endif // __BL_SSI_H__
