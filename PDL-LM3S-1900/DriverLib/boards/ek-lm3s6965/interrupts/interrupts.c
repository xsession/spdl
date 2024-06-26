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
#include "../osram128x64x4.h"

//*****************************************************************************
//
//! \addtogroup ek_lm3s6965_list
//! <h1>Interrupts (interrupts)</h1>
//!
//! This example application demonstrates the interrupt preemption and
//! tail-chaining capabilities of Cortex-M3 microprocessor and NVIC.  Nested
//! interrupts are synthesized when the interrupts have the same priority,
//! increasing priorities, and decreasing priorities.  With increasing
//! priorities, preemption will occur; in the other two cases tail-chaining
//! will occur.  The currently pending interrupts and the currently executing
//! interrupt will be displayed on the OLED; GPIO pins B0, B1 and B2 will be
//! asserted upon interrupt handler entry and de-asserted before interrupt
//! handler exit so that the off-to-on time can be observed with a scope or
//! logic analyzer to see the speed of tail-chaining (for the two cases where
//! tail-chaining is occurring).
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
// Display the interrupt state on the OLED.  The currently active and pending
// interrupts are displayed.
//
//*****************************************************************************
void
DisplayIntStatus(void)
{
    unsigned long ulTemp;
    char pcBuffer[4];

    //
    // Display the currently active interrupts.
    //
    ulTemp = HWREG(NVIC_ACTIVE0);
    pcBuffer[0] = (ulTemp & 1) ? '1' : ' ';
    pcBuffer[1] = (ulTemp & 2) ? '2' : ' ';
    pcBuffer[2] = (ulTemp & 4) ? '3' : ' ';
    pcBuffer[3] = '\0';
    OSRAM128x64x4StringDraw(pcBuffer, 42, 40, 15);

    //
    // Display the currently pending interrupts.
    //
    ulTemp = HWREG(NVIC_PEND0);
    pcBuffer[0] = (ulTemp & 1) ? '1' : ' ';
    pcBuffer[1] = (ulTemp & 2) ? '2' : ' ';
    pcBuffer[2] = (ulTemp & 4) ? '3' : ' ';
    OSRAM128x64x4StringDraw(pcBuffer, 96, 40, 15);
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
    // Set PF0 high to indicate entry to this interrupt handler.
    //
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);

    //
    // Put the current interrupt state on the OLED.
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
    // Set PF0 low to indicate exit from this interrupt handler.
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
    // Set PD1 high to indicate entry to this interrupt handler.
    //
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1);

    //
    // Put the current interrupt state on the OLED.
    //
    DisplayIntStatus();

    //
    // Trigger the INT_GPIOA interrupt.
    //
    HWREG(NVIC_SW_TRIG) = INT_GPIOA - 16;

    //
    // Put the current interrupt state on the OLED.
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
    // Set PD1 low to indicate exit from this interrupt handler.
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
    // Set PD2 high to indicate entry to this interrupt handler.
    //
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);

    //
    // Put the current interrupt state on the OLED.
    //
    DisplayIntStatus();

    //
    // Trigger the INT_GPIOB interrupt.
    //
    HWREG(NVIC_SW_TRIG) = INT_GPIOB - 16;

    //
    // Put the current interrupt state on the OLED.
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
    // Set PD2 low to indicate exit from this interrupt handler.
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
                   SYSCTL_XTAL_8MHZ);

    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //
    // Initialize the OLED display and write status.
    //
    OSRAM128x64x4Init(1000000);
    OSRAM128x64x4StringDraw("Act:    Pend:   ", 18, 40, 15);

    //
    // Configure the F0, D1 and D2 to be outputs to indicate entry/exit of one
    // of the interrupt handlers.
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
    OSRAM128x64x4StringDraw("Equal Priority  ", 18, 24, 15);

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
    OSRAM128x64x4StringDraw("Dec. Priority   ", 18, 24, 15);

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
    // Put the current interrupt state on the OLED.
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
    OSRAM128x64x4StringDraw("Inc. Priority   ", 18, 24, 15);

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
    // Put the current interrupt state on the OLED.
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
    OSRAM128x64x4StringDraw("Int Priority    ", 18, 24, 15);
    if(ulError)
    {
        OSRAM128x64x4StringDraw("=: P  >: P  <: P", 18, 40, 15);
        if(ulError & 1)
        {
            OSRAM128x64x4StringDraw("F", 36, 40, 15);
        }
        if(ulError & 2)
        {
            OSRAM128x64x4StringDraw("F", 72, 40, 15);
        }
        if(ulError & 4)
        {
            OSRAM128x64x4StringDraw("F", 108, 40, 15);
        }
    }
    else
    {
        OSRAM128x64x4StringDraw("Success.        ", 18, 40, 15);
    }

    //
    // Exit.
    //
    DiagExit(0);
}
