//*****************************************************************************
//
// interrupts.c - Interrupt preemption and tail-chaining example.
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
#include "../../../hw_nvic.h"
#include "../../../hw_types.h"
#include "../../../src/debug.h"
#include "../../../src/gpio.h"
#include "../../../src/interrupt.h"
#include "../../../src/systick.h"
#include "../../../src/sysctl.h"
#include "../../../utils/diag.h"
#include "../pdc.h"

//*****************************************************************************
//
//! \addtogroup dk_lm3s828_list
//! <h1>Interrupts (interrupts)</h1>
//!
//! This example application demonstrates the interrupt preemption and
//! tail-chaining capabilities of Cortex-M3 microprocessor and NVIC.  Nested
//! interrupts are synthesized when the interrupts have the same priority,
//! increasing priorities, and decreasing priorities.  With increasing
//! priorities, preemption will occur; in the other two cases tail-chaining
//! will occur.  The currently pending interrupts and the currently executing
//! interrupt will be displayed on the LCD; individual LEDs connected to port
//! B0-B2 will be turned on upon interrupt handler entry and off before
//! interrupt handler exit so that the off-to-on time can be observed with a
//! scope or logic analyzer to see the speed of tail-chaining (for the two
//! cases where tail-chaining is occurring).
//!
//! In order for this example to work properly, the ULED0 (JP22), ULED1 (JP23),
//! and ULED2 (JP24) jumpers must be installed on the board, and the PB1
//! (JP1) jumper on the daughtercard must be set to pins 2 & 3.
//
//*****************************************************************************

//*****************************************************************************
//
// The count of interrupts received.  This is incremented as each interrupt
// handler runs, and its value saved into interrupt handler specific values to
// determine the order in which the interrupt handlers were executed.
//
//*****************************************************************************
volatile unsigned long g_ulIndex;

//*****************************************************************************
//
// The value of g_ulIndex when the INT_GPIOA interrupt was processed.
//
//*****************************************************************************
volatile unsigned long g_ulGPIOa;

//*****************************************************************************
//
// The value of g_ulIndex when the INT_GPIOB interrupt was processed.
//
//*****************************************************************************
volatile unsigned long g_ulGPIOb;

//*****************************************************************************
//
// The value of g_ulIndex when the INT_GPIOC interrupt was processed.
//
//*****************************************************************************
volatile unsigned long g_ulGPIOc;

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
// Delay for the specified number of seconds.  Depending upon the current
// SysTick value, the delay will be between N-1 and N seconds (i.e. N-1 full
// seconds are guaranteed, along with the remainder of the current second).
//
//*****************************************************************************
void
Delay(unsigned long ulSeconds)
{
    //
    // Loop while there are more seconds to wait.
    //
    while(ulSeconds--)
    {
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

//*****************************************************************************
//
// Display the interrupt state on the LCD.  The currently active and pending
// interrupts are displayed.
//
//*****************************************************************************
void
DisplayIntStatus(void)
{
    unsigned long ulTemp;

    //
    // Display the currently active interrupts.
    //
    ulTemp = HWREG(NVIC_ACTIVE0);
    PDCLCDSetPos(4, 1);
    PDCLCDWrite((ulTemp & 1) ? "1" : " ", 1);
    PDCLCDWrite((ulTemp & 2) ? "2" : " ", 1);
    PDCLCDWrite((ulTemp & 4) ? "3" : " ", 1);

    //
    // Display the currently pending interrupts.
    //
    ulTemp = HWREG(NVIC_PEND0);
    PDCLCDSetPos(13, 1);
    PDCLCDWrite((ulTemp & 1) ? "1" : " ", 1);
    PDCLCDWrite((ulTemp & 2) ? "2" : " ", 1);
    PDCLCDWrite((ulTemp & 4) ? "3" : " ", 1);
}

//*****************************************************************************
//
// This is the handler for INT_GPIOA.  It simply saves the interrupt sequence
// number.
//
//*****************************************************************************
void
IntGPIOa(void)
{
    //
    // Set PB0 high to indicate entry to this interrupt handler.
    //
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);

    //
    // Put the current interrupt state on the LCD.
    //
    DisplayIntStatus();

    //
    // Wait two seconds.
    //
    Delay(2);

    //
    // Save and increment the interrupt sequence number.
    //
    g_ulGPIOa = g_ulIndex++;

    //
    // Set PB0 low to indicate exit from this interrupt handler.
    //
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0);
}

//*****************************************************************************
//
// This is the handler for INT_GPIOB.  It triggers INT_GPIOA and saves the
// interrupt sequence number.
//
//*****************************************************************************
void
IntGPIOb(void)
{
    //
    // Set PB1 high to indicate entry to this interrupt handler.
    //
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);

    //
    // Put the current interrupt state on the LCD.
    //
    DisplayIntStatus();

    //
    // Trigger the INT_GPIOA interrupt.
    //
    HWREG(NVIC_SW_TRIG) = INT_GPIOA - 16;

    //
    // Put the current interrupt state on the LCD.
    //
    DisplayIntStatus();

    //
    // Wait two seconds.
    //
    Delay(2);

    //
    // Save and increment the interrupt sequence number.
    //
    g_ulGPIOb = g_ulIndex++;

    //
    // Set PB1 low to indicate exit from this interrupt handler.
    //
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0);
}

//*****************************************************************************
//
// This is the handler for INT_GPIOC.  It triggers INT_GPIOB and saves the
// interrupt sequence number.
//
//*****************************************************************************
void
IntGPIOc(void)
{
    //
    // Set PB2 high to indicate entry to this interrupt handler.
    //
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);

    //
    // Put the current interrupt state on the LCD.
    //
    DisplayIntStatus();

    //
    // Trigger the INT_GPIOB interrupt.
    //
    HWREG(NVIC_SW_TRIG) = INT_GPIOB - 16;

    //
    // Put the current interrupt state on the LCD.
    //
    DisplayIntStatus();

    //
    // Wait two seconds.
    //
    Delay(2);

    //
    // Save and increment the interrupt sequence number.
    //
    g_ulGPIOc = g_ulIndex++;

    //
    // Set PB2 low to indicate exit from this interrupt handler.
    //
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);
}

//*****************************************************************************
//
// This is the main example program.  It checks to see that the interrupts are
// processed in the correct order when they have identical priorities,
// increasing priorities, and decreasing priorities.  This exercises interrupt
// preemption and tail chaining.
//
//*****************************************************************************
int
main(void)
{
    unsigned long ulError;

    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_6MHZ);

    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //
    // Init the PDC, and then the LCD, then write the LEDs.
    //
    PDCInit();
    PDCLCDInit();
    PDCLCDBacklightOn();
    PDCLCDSetPos(0, 1);
    PDCLCDWrite("Act:    Pend:   ", 16);

    //
    // Configure the first three pins of GPIO port B to be outputs to indicate
    // entry/exit of one of the interrupt handlers.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,
                          GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, 0);

    //
    // Set up and enable the SysTick timer.  It will be used as a reference
    // for delay loops in the interrupt handlers.  The SysTick timer period
    // will be set up for one second.
    //
    SysTickPeriodSet(SysCtlClockGet());
    SysTickEnable();

    //
    // Reset the error indicator.
    //
    ulError = 0;

    //
    // Enable interrupts to the processor.
    //
    IntMasterEnable();

    //
    // Enable the interrupts.
    //
    IntEnable(INT_GPIOA);
    IntEnable(INT_GPIOB);
    IntEnable(INT_GPIOC);

    //
    // Indicate that the equal interrupt priority test is beginning.
    //
    PDCLCDSetPos(0, 0);
    PDCLCDWrite("Equal Priority  ", 16);

    //
    // Set the interrupt priorities so they are all equal.
    //
    IntPrioritySet(INT_GPIOA, 0x00);
    IntPrioritySet(INT_GPIOB, 0x00);
    IntPrioritySet(INT_GPIOC, 0x00);

    //
    // Reset the interrupt flags.
    //
    g_ulGPIOa = 0;
    g_ulGPIOb = 0;
    g_ulGPIOc = 0;
    g_ulIndex = 1;

    //
    // Trigger the interrupt for GPIO C.
    //
    HWREG(NVIC_SW_TRIG) = INT_GPIOC - 16;

    //
    // Put the current interrupt state on the LCD.
    //
    DisplayIntStatus();

    //
    // Verify that the interrupts were processed in the correct order.
    //
    if((g_ulGPIOa != 3) || (g_ulGPIOb != 2) || (g_ulGPIOc != 1))
    {
        ulError |= 1;
    }

    //
    // Wait two seconds.
    //
    Delay(2);

    //
    // Indicate that the decreasing interrupt priority test is beginning.
    //
    PDCLCDSetPos(0, 0);
    PDCLCDWrite("Dec. Priority   ", 16);

    //
    // Set the interrupt priorities so that they are decreasing (i.e. C > B >
    // A).
    //
    IntPrioritySet(INT_GPIOA, 0x80);
    IntPrioritySet(INT_GPIOB, 0x40);
    IntPrioritySet(INT_GPIOC, 0x00);

    //
    // Reset the interrupt flags.
    //
    g_ulGPIOa = 0;
    g_ulGPIOb = 0;
    g_ulGPIOc = 0;
    g_ulIndex = 1;

    //
    // Trigger the interrupt for GPIO C.
    //
    HWREG(NVIC_SW_TRIG) = INT_GPIOC - 16;

    //
    // Put the current interrupt state on the LCD.
    //
    DisplayIntStatus();

    //
    // Verify that the interrupts were processed in the correct order.
    //
    if((g_ulGPIOa != 3) || (g_ulGPIOb != 2) || (g_ulGPIOc != 1))
    {
        ulError |= 2;
    }

    //
    // Wait two seconds.
    //
    Delay(2);

    //
    // Indicate that the increasing interrupt priority test is beginning.
    //
    PDCLCDSetPos(0, 0);
    PDCLCDWrite("Inc. Priority   ", 16);

    //
    // Set the interrupt priorities so that they are increasing (i.e. C < B <
    // A).
    //
    IntPrioritySet(INT_GPIOA, 0x00);
    IntPrioritySet(INT_GPIOB, 0x40);
    IntPrioritySet(INT_GPIOC, 0x80);

    //
    // Reset the interrupt flags.
    //
    g_ulGPIOa = 0;
    g_ulGPIOb = 0;
    g_ulGPIOc = 0;
    g_ulIndex = 1;

    //
    // Trigger the interrupt for GPIO C.
    //
    HWREG(NVIC_SW_TRIG) = INT_GPIOC - 16;

    //
    // Put the current interrupt state on the LCD.
    //
    DisplayIntStatus();

    //
    // Verify that the interrupts were processed in the correct order.
    //
    if((g_ulGPIOa != 1) || (g_ulGPIOb != 2) || (g_ulGPIOc != 3))
    {
        ulError |= 4;
    }

    //
    // Wait two seconds.
    //
    Delay(2);

    //
    // Disable the interrupts.
    //
    IntDisable(INT_GPIOA);
    IntDisable(INT_GPIOB);
    IntDisable(INT_GPIOC);

    //
    // Disable interrupts to the processor.
    //
    IntMasterDisable();

    //
    // Print out the test results.
    //
    PDCLCDSetPos(0, 0);
    PDCLCDWrite("Int Priority    ", 16);
    PDCLCDSetPos(0, 1);
    if(ulError)
    {
        PDCLCDWrite("=: P  >: P  <: P", 16);
        PDCLCDSetPos(3, 1);
        PDCLCDWrite((ulError & 1) ? "F" : "P", 1);
        PDCLCDSetPos(9, 1);
        PDCLCDWrite((ulError & 2) ? "F" : "P", 1);
        PDCLCDSetPos(15, 1);
        PDCLCDWrite((ulError & 4) ? "F" : "P", 1);
    }
    else
    {
        PDCLCDWrite("Success.        ", 16);
    }

    //
    // Exit.
    //
    DiagExit(0);
}
