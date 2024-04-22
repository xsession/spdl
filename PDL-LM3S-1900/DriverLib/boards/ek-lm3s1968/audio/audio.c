//*****************************************************************************
//
// audio.c - Audio playback example.
//
// Copyright (c) 2007 Luminary Micro, Inc.  All rights reserved.
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

#include "../../../hw_memmap.h"
#include "../../../hw_sysctl.h"
#include "../../../hw_types.h"
#include "../../../src/gpio.h"
#include "../../../src/sysctl.h"
#include "../../../src/systick.h"
#include "../rit128x96x4.h"
#include "../class-d.h"
#include "adpcm.h"
#include "pcm.h"

//*****************************************************************************
//
//! \addtogroup ek_lm3s1968_list
//! <h1>Audio Playback (audio)</h1>
//!
//! This example application plays audio via the Class-D amplifier and speaker.
//! The audio is provided in both PCM and ADPCM format so that the audio
//! quality can be compared.
//
//*****************************************************************************

//*****************************************************************************
//
// The debounced state of the five push buttons.  The bit positions correspond
// to:
//
//     3 - Up
//     4 - Down
//     5 - Left
//     6 - Right
//     7 - Select
//
//*****************************************************************************
unsigned char g_ucSwitches = 0xf8;

//*****************************************************************************
//
// The vertical counter used to debounce the push buttons.  The bit positions
// are the same as g_ucSwitches.
//
//*****************************************************************************
static unsigned char g_ucSwitchClockA = 0;
static unsigned char g_ucSwitchClockB = 0;

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
// Handles the SysTick timeout interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    unsigned long ulData, ulDelta;

    //
    // Read the state of the buttons.
    //
    ulData = GPIOPinRead(GPIO_PORTG_BASE,
                         (GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                          GPIO_PIN_7));

    //
    // Determine the switches that are at a different state than the debounced
    // state.
    //
    ulDelta = ulData ^ g_ucSwitches;

    //
    // Increment the clocks by one.
    //
    g_ucSwitchClockA ^= g_ucSwitchClockB;
    g_ucSwitchClockB = ~g_ucSwitchClockB;

    //
    // Reset the clocks corresponding to switches that have not changed state.
    //
    g_ucSwitchClockA &= ulDelta;
    g_ucSwitchClockB &= ulDelta;

    //
    // Get the new debounced switch state.
    //
    g_ucSwitches &= g_ucSwitchClockA | g_ucSwitchClockB;
    g_ucSwitches |= (~(g_ucSwitchClockA | g_ucSwitchClockB)) & ulData;

    //
    // Determine the switches that just changed debounced state.
    //
    ulDelta ^= (g_ucSwitchClockA | g_ucSwitchClockB);

    //
    // See if the up button was just pressed.
    //
    if((ulDelta & 0x08) && !(g_ucSwitches & 0x08))
    {
        //
        // Increase the volume of the playback.
        //
        ClassDVolumeUp(16);
    }

    //
    // See if the down button was just pressed.
    //
    if((ulDelta & 0x10) && !(g_ucSwitches & 0x10))
    {
        //
        // Decrease the volume of the playback.
        //
        ClassDVolumeDown(16);
    }

    //
    // See if the left button was just pressed.
    //
    if((ulDelta & 0x20) && !(g_ucSwitches & 0x20))
    {
        //
        // Start playback of the PCM stream.
        //
        ClassDPlayPCM(g_pucPCMData, sizeof(g_pucPCMData));
    }

    //
    // See if the right button was just pressed.
    //
    if((ulDelta & 0x40) && !(g_ucSwitches & 0x40))
    {
        //
        // Start playback of the ADPCM stream.
        //
        ClassDPlayADPCM(g_pucADPCMData, sizeof(g_pucADPCMData));
    }

    //
    // See if the select button was just pressed.
    //
    if((ulDelta & 0x80) && !(g_ucSwitches & 0x80))
    {
        //
        // Stop playback.
        //
        ClassDStop();
    }
}

//*****************************************************************************
//
// This example demonstrates the use of bit-banding to set individual bits
// within a word of SRAM.
//
//*****************************************************************************
int
main(void)
{
    //
    // If running on Rev A2 silicon, turn the LDO voltage up to 2.75V.  This is
    // a workaround to allow the PLL to operate reliably.
    //
    if(DEVICE_IS_REVA2)
    {
        SysCtlLDOSet(SYSCTL_LDO_2_75V);
    }

    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    //
    // Enable the peripherals used by the application.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

    //
    // Configure the GPIOs used to read the state of the on-board push buttons.
    //
    GPIOPinTypeGPIOInput(GPIO_PORTG_BASE,
                         (GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                          GPIO_PIN_7));
    GPIOPadConfigSet(GPIO_PORTG_BASE,
                     (GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 |
                      GPIO_PIN_7), GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    //
    // Initialize the OLED display and print out the directions.
    //
    RIT128x96x4Init(1000000);
    RIT128x96x4StringDraw("Audio Playback via", 10, 16, 15);
    RIT128x96x4StringDraw("Class-D Amplifier", 12, 24, 15);
    RIT128x96x4StringDraw("Press Up for Vol+", 12, 40, 15);
    RIT128x96x4StringDraw("Press Down for Vol-", 6, 48, 15);
    RIT128x96x4StringDraw("Press Left for PCM", 10, 56, 15);
    RIT128x96x4StringDraw("Press Right for ADPCM", 0, 64, 15);
    RIT128x96x4StringDraw("Press Select to stop", 4, 72, 15);

    //
    // Initialize the Class-D amplifier driver.
    //
    ClassDInit(SysCtlClockGet());

    //
    // Wait until the Class-D amplifier driver is done starting up.
    //
    while(ClassDBusy())
    {
    }

    //
    // Start playback of the PCM stream.
    //
    ClassDPlayPCM(g_pucPCMData, sizeof(g_pucPCMData));

    //
    // Set up and enable SysTick.
    //
    SysTickPeriodSet(SysCtlClockGet() / 200);
    SysTickEnable();
    SysTickIntEnable();

    //
    // Loop forever.
    //
    while(1)
    {
    }
}
