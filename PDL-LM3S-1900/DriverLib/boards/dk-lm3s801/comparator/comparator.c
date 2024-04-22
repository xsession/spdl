//*****************************************************************************
//
// comparator.c - Analog comparator example.
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
#include "../../../src/comp.h"
#include "../../../src/debug.h"
#include "../../../src/gpio.h"
#include "../../../src/interrupt.h"
#include "../../../src/sysctl.h"
#include "../../../utils/diag.h"
#include "../pdc.h"

//*****************************************************************************
//
//! \addtogroup dk_lm3s801_list
//! <h1>Comparator (comparator)</h1>
//!
//! This example application demonstrates the operation of the analog
//! comparator(s).  Comparator zero (which is present on all devices that have
//! analog comparators) is configured to compare its negative input to an
//! internally generated 1.65 V reference and toggle the state of the LED on
//! port B0 based on comparator change interrupts.  The LED will be turned on
//! by the interrupt handler when a rising edge on the comparator output is
//! detected, and will be turned off when a falling edge is detected.
//!
//! In order for this example to work properly, the ULED0 (JP22) jumper must
//! be installed on the board.
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
// The interrupt handler for the comparator interrupt.
//
//*****************************************************************************
void
CompIntHandler(void)
{
    //
    // Clear the comparator interrupt.
    //
    ComparatorIntClear(COMP_BASE, 0);

    //
    // Set the GPIO B0 value based on the current output value of the
    // comparator.
    //
    if(ComparatorValueGet(COMP_BASE, 0) == true)
    {
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);
    }
    else
    {
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0);
    }
}

//*****************************************************************************
//
// This example demonstrates how to setup an analog comparator and trigger
// interrupts on output changes.
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_6MHZ);

    //
    // Initialize the peripheral device controller/LCD.
    //
    PDCInit();
    PDCLCDInit();
    PDCLCDBacklightOn();

    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_COMP0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Set GPIO B0 as an output.  This drives an LED on the board that will
    // indicate the current comparator output value.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);

    //
    // Configure the C0- input.
    //
    GPIOPinTypeComparator(GPIO_PORTB_BASE, GPIO_PIN_4);

    //
    // Set the internal reference voltage generator to 1.65 V.
    //
    ComparatorRefSet(COMP_BASE, COMP_REF_1_65V);

    //
    // Configure the comparator to use the internal reference voltage and to
    // interrupt on rising and falling edges of the output.
    //
    ComparatorConfigure(COMP_BASE, 0, COMP_INT_BOTH | COMP_ASRCP_REF);

    //
    // Enable the comparator interrupt.
    //
    IntEnable(INT_COMP0);
    ComparatorIntEnable(COMP_BASE, 0);

    //
    // Clear the screen and tell the user what to do.
    //
    PDCLCDClear();
    PDCLCDSetPos(0, 0);
    PDCLCDWrite("Turn the Pot -->", 16);

    //
    // Loop forever while the LED tracks the comparator value.
    //
    while(1)
    {
    }
}
