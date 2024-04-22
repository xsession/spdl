//*****************************************************************************
//
// gpio_led.c - GPIO example that lights up the LEDs in an amusing fashion.
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

#include "../../../hw_memmap.h"
#include "../../../hw_types.h"
#include "../../../src/debug.h"
#include "../../../src/gpio.h"
#include "../../../src/sysctl.h"
#include "../../../src/systick.h"
#include "../pdc.h"

//*****************************************************************************
//
//! \addtogroup dk_lm3s818_list
//! <h1>GPIO (gpio_led)</h1>
//!
//! This example application uses LEDs connected to GPIO pins to create a
//! ``roving eye'' display.  Port B0-B3 are driven in a sequential manner to
//! give the illusion of an eye looking back and forth.
//!
//! In order for this example to work properly, the ULED0 (JP22), ULED1 (JP23),
//! ULED2 (JP24), and ULED3 (JP25) jumpers must be installed on the board, and
//! the PB1 (JP1) jumper on the daughtercard must be set to pins 2 & 3.
//
//*****************************************************************************

//*****************************************************************************
//
// The set of GPIO pins that will be used for the LED show.
//
//*****************************************************************************
#define PINS                    (GPIO_PIN_0 | GPIO_PIN_1 | \
                                 GPIO_PIN_2 | GPIO_PIN_3)

//*****************************************************************************
//
// The sequence of LED patterns in the show.
//
//*****************************************************************************
static const unsigned long g_ulLEDs[] =
{
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_2, GPIO_PIN_1
};

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
// This example demonstrates the use of the GPIO outputs.
//
//*****************************************************************************
int
main(void)
{
    unsigned long ulIdx;

    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_6MHZ);

    //
    // Init the PDC, and then the LCD, then write the LCD.
    //
    PDCInit();
    PDCLCDInit();
    PDCLCDBacklightOn();
    PDCLCDSetPos(1, 0);
    PDCLCDWrite("Toggling GPIOs", 14);

    //
    // Enable the GPIO block.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //
    // Set GPIO B0 as an output.  This drives an LED on the board that will
    // toggle when a watchdog interrupt is processed.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, PINS);

    //
    // Set up and enable the SysTick timer.  It will be used as a reference
    // for the delay loop.  The SysTick timer period will be set up for twelve
    // interrupts per second.
    //
    SysTickPeriodSet(SysCtlClockGet() / 12);
    SysTickEnable();

    //
    // Loop forever cycling the outputs to the LEDs.
    //
    for(ulIdx = 0; ; ulIdx = (ulIdx + 1) % 6)
    {
        //
        // Set the LEDs to the current value.
        //
        GPIOPinWrite(GPIO_PORTB_BASE, PINS, g_ulLEDs[ulIdx]);

        //
        // Wait until the SysTick value is less than 1000.
        //
        while(SysTickValueGet() > 1000)
        {
        }

        //
        // Wait until the SysTick value is greater than 1000.
        //
        while(SysTickValueGet() < 1000)
        {
        }
    }
}
