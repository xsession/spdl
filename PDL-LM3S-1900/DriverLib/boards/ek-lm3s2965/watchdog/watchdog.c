//*****************************************************************************
//
// watchdog.c - Watchdog timer example.
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
#include "../../../src/sysctl.h"
#include "../../../src/watchdog.h"
#include "../osram128x64x4.h"

//*****************************************************************************
//
//! \addtogroup ek_lm3s2965_list
//! <h1>Watchdog (watchdog)</h1>
//!
//! This example application demonstrates the use of the watchdog as a simple
//! heartbeat for the system.  If the watchdog is not periodically fed, it will
//! reset the system.  Each time the watchdog is fed, the LED is inverted so
//! that it is easy to see that it is being fed, which occurs once every
//! second.
//
//*****************************************************************************

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
// The interrupt handler for the watchdog.  This feeds the dog (so that the
// processor does not get reset) and winks the LED connected to GPIO B0.
//
//*****************************************************************************
void
WatchdogIntHandler(void)
{
    //
    // Clear the watchdog interrupt.
    //
    WatchdogIntClear(WATCHDOG_BASE);

    //
    // Invert the GPIO G2 value.
    //
    GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2,
                 GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_2) ^ GPIO_PIN_2);
}

//*****************************************************************************
//
// This example demonstrates the use of the watchdog timer.
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);

    //
    // Initialize the OLED display and write status.
    //
    OSRAM128x64x4Init(1000000);
    OSRAM128x64x4StringDraw("Watchdog example", 12, 24, 15);

    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Set GPIO G2 as an output.  This drives an LED on the board that will
    // toggle when a watchdog interrupt is processed.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, 0);

    //
    // Enable the watchdog interrupt.
    //
    IntEnable(INT_WATCHDOG);

    //
    // Set the period of the watchdog timer.
    //
    WatchdogReloadSet(WATCHDOG_BASE, SysCtlClockGet());

    //
    // Enable reset generation from the watchdog timer.
    //
    WatchdogResetEnable(WATCHDOG_BASE);

    //
    // Enable the watchdog timer.
    //
    WatchdogEnable(WATCHDOG_BASE);

    //
    // Loop forever while the LED winks as watchdog interrupts are handled.
    //
    while(1)
    {
    }
}
