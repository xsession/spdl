//*****************************************************************************
//
// boot_demo2.c - Second boot loader example.
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
#include "../../../hw_types.h"
#include "../../../src/debug.h"
#include "../../../src/gpio.h"
#include "../../../src/sysctl.h"
#include "../../../src/uart.h"
#include "../rit128x96x4.h"

//*****************************************************************************
//
//! \addtogroup ek_lm3s2965_revc_list
//! <h1>Boot Loader Demo 2 (boot_demo2)</h1>
//!
//! An example to demonstrate the use of the boot loader.  After being started
//! by the boot loader, the application will configure the UART, wait for
//! select button to be pressed, and then branch back to the boot loader to
//! await the start of an update.  The UART will always be configured at
//! 115,200 baud and does not require the use of auto-bauding.
//!
//! Both the boot loader and the application must be placed into flash.  Once
//! the boot loader is in flash, it can be used to program the application into
//! flash as well.  Then, the boot loader can be used to replace the
//! application with another.
//!
//! The boot_demo1 application can be used along with this application to
//! easily demonstrate that the boot loader is actually updating the on-chip
//! flash.
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
// Demonstrate the use of the boot loader.
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
    // Enable the UART and GPIO modules.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Make the UART pins be peripheral controlled.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Configure the UART for 115,200, 8-N-1 operation.
    //
    UARTConfigSetExpClk(UART0_BASE, 8000000, 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    //
    // Initialize the OLED display.
    //
    RIT128x96x4Init(1000000);

    //
    // Indicate what is happening.
    //
    RIT128x96x4StringDraw("Boot Loader Demo Two", 4, 20, 15);
    RIT128x96x4StringDraw("Press the select", 16, 44, 15);
    RIT128x96x4StringDraw("button to start the", 6, 52, 15);
    RIT128x96x4StringDraw("boot loader.", 28, 60, 15);

    //
    // Enable the GPIO pin to read the select button.
    //
    GPIODirModeSet(GPIO_PORTG_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(GPIO_PORTG_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);

    //
    // Wait until the select button has been pressed.
    //
    while(GPIOPinRead(GPIO_PORTG_BASE, GPIO_PIN_4) != 0)
    {
    }

    //
    // Drain any data that may be in the UART fifo.
    //
    while(UARTCharsAvail(UART0_BASE))
    {
        UARTCharGet(UART0_BASE);
    }

    //
    // Indicate that the boot loader is being called.
    //
    RIT128x96x4StringDraw("The boot loader is", 10, 36, 15);
    RIT128x96x4StringDraw(" now running and ", 12, 44, 15);
    RIT128x96x4StringDraw(" awaiting an update ", 4, 52, 15);
    RIT128x96x4StringDraw("over UART0 at", 24, 60, 15);
    RIT128x96x4StringDraw("115,200, 8-N-1.", 18, 68, 15);

    //
    // Call the boot loader so that it will listen for an update on the UART.
    //
    (*((void (*)(void))(*(unsigned long *)0x2c)))();
}
