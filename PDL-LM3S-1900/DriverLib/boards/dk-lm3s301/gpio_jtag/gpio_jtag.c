//*****************************************************************************
//
// gpio_jtag.c - Example to demonstrate recovering the JTAG interface.
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

#include "../../../hw_ints.h"
#include "../../../hw_memmap.h"
#include "../../../hw_types.h"
#include "../../../src/debug.h"
#include "../../../src/gpio.h"
#include "../../../src/interrupt.h"
#include "../../../src/sysctl.h"
#include "../pdc.h"

//*****************************************************************************
//
//! \addtogroup dk_lm3s301_list
//! <h1>GPIO JTAG Recovery (gpio_jtag)</h1>
//!
//! This example demonstrates changing the JTAG pins into GPIOs, along with a
//! mechanism to revert them to JTAG pins.  When first run, the pins remain in
//! JTAG mode.  Pressing the user push button will toggle the pins between JTAG
//! mode and GPIO mode.  Because there is no debouncing of the push button
//! (either in hardware or software), a button press will occasionally result
//! in more than one mode change.
//!
//! In this example, all five pins (PB7, PC0, PC1, PC2, and PC3) are switched,
//! though the more typical use would be to change PB7 into a GPIO.  Note that
//! because of errata in Rev Bx and Rev C0 of Sandstorm-class Stellaris
//! microcontrollers, JTAG and SWD will not function if PB7 is configured as a
//! GPIO.  This errata is fixed in Rev C2 of Sandstorm-class Stellaris
//! microcontrollers.
//
//*****************************************************************************

//*****************************************************************************
//
// The current mode of pins PB7, PC0, PC1, PC2, and PC3.  When zero, the pins
// are in JTAG mode; when non-zero, the pins are in GPIO mode.
//
//*****************************************************************************
volatile unsigned long g_ulMode;

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
// The interrupt handler for the PB4 pin interrupt.  When triggered, this will
// toggle the JTAG pins from between JTAG and GPIO mode.
//
//*****************************************************************************
void
GPIOBIntHandler(void)
{
    //
    // Clear the GPIO interrupt.
    //
    GPIOPinIntClear(GPIO_PORTB_BASE, GPIO_PIN_4);

    //
    // Toggle the pin mode.
    //
    g_ulMode ^= 1;

    //
    // See if the pins should be in JTAG or GPIO mode.
    //
    if(g_ulMode == 0)
    {
        //
        // Change PB7 and PC0-3 into hardware (i.e. JTAG) pins.
        //
        GPIODirModeSet(GPIO_PORTB_BASE, GPIO_PIN_7, GPIO_DIR_MODE_HW);
        GPIODirModeSet(GPIO_PORTC_BASE,
                       GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                       GPIO_DIR_MODE_HW);
    }
    else
    {
        //
        // Change PB7 and PC0-3 into GPIO inputs.
        //
        GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_7);
        GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, (GPIO_PIN_0 | GPIO_PIN_1 |
                                               GPIO_PIN_2 | GPIO_PIN_3));
    }
}

//*****************************************************************************
//
// Toggle the JTAG pins between JTAG and GPIO mode with a push button selecting
// between the two.
//
//*****************************************************************************
int
main(void)
{
    unsigned long ulMode;

    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_6MHZ);

    //
    // Enable the peripherals used by this application.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    //
    // Configure the push button as an input and enable the pin to interrupt on
    // the falling edge (i.e. when the push button is pressed).
    //
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_4);
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
    GPIOPinIntEnable(GPIO_PORTB_BASE, GPIO_PIN_4);
    IntEnable(INT_GPIOB);

    //
    // Set the global and local indicator of pin mode to zero, meaning JTAG.
    //
    g_ulMode = 0;
    ulMode = 0;

    //
    // Init the PDC and then the LCD.
    //
    PDCInit();
    PDCLCDInit();
    PDCLCDBacklightOn();
    PDCLCDSetPos(1, 0);
    PDCLCDWrite("PB7/PC0-3 are", 13);
    PDCLCDSetPos(5, 1);
    PDCLCDWrite("JTAG", 4);

    //
    // Loop forever.  This loop simply exists to display on the LCD the current
    // state of PB7/PC0-3; the handling of changing the JTAG pins to and from
    // GPIO mode is done in GPIOBIntHandler().
    //
    while(1)
    {
        //
        // Wait until the pin mode changes.
        //
        while(g_ulMode == ulMode)
        {
        }

        //
        // Save the new mode locally so that a subsequent pin mode change can
        // be detected.
        //
        ulMode = g_ulMode;

        //
        // See what the new pin mode was changed to.
        //
        if(ulMode == 0)
        {
            //
            // Indicate that PB7 and PC0-3 are currently JTAG pins.
            //
            PDCLCDSetPos(5, 1);
            PDCLCDWrite("JTAG", 4);
        }
        else
        {
            //
            // Indicate that PB7 and PC0-3 are currently GPIO pins.
            //
            PDCLCDSetPos(5, 1);
            PDCLCDWrite("GPIO", 4);
        }
    }
}
