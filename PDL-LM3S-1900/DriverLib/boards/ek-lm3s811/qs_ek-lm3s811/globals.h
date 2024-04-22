//*****************************************************************************
//
// globals.h - Shared configuration and global variables.
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

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

//*****************************************************************************
//
// The rate at which the potentiometer is sampled, which is also the update
// period for the game itself.
//
//*****************************************************************************
#define CLOCK_RATE              120

//*****************************************************************************
//
// The GPIOs for the push button and the user LED.
//
//*****************************************************************************
#define PUSH_BUTTON             GPIO_PIN_4
#define USER_LED                GPIO_PIN_5

//*****************************************************************************
//
// Get a new random number via a fast linear congruence generator.
//
//*****************************************************************************
#define NEXT_RAND(x)            (((x) * 1664525) + 1013904223)

//*****************************************************************************
//
// A set of flags used to track the state of the application.
//
//*****************************************************************************
extern unsigned long g_ulFlags;
#define FLAG_CLOCK_TICK         0           // A timer interrupt has occurred
#define FLAG_CLOCK_COUNT_LOW    1           // The low bit of the clock count
#define FLAG_CLOCK_COUNT_HIGH   2           // The high bit of the clock count
#define FLAG_UPDATE             3           // The display should be updated
#define FLAG_BUTTON             4           // Debounced state of the button
#define FLAG_DEBOUNCE_LOW       5           // Low bit of the debounce clock
#define FLAG_DEBOUNCE_HIGH      6           // High bit of the debounce clock
#define FLAG_BUTTON_PRESS       7           // The button was just pressed

//*****************************************************************************
//
// The current filtered value of the potentiometer.
//
//*****************************************************************************
extern unsigned long g_ulWheel;

//*****************************************************************************
//
// Storage for a local frame buffer.
//
//*****************************************************************************
extern unsigned char g_pucFrame[192];

//*****************************************************************************
//
// Storage for the background image of the tunnel.  This is copied to the local
// frame buffer and then the other elements are overlaid upon it.
//
//*****************************************************************************
extern unsigned char g_pucBackground[192];

#endif // __GLOBALS_H__
