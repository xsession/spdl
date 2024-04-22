//*****************************************************************************
//
// qs_dk-lm3s828.c - A quick start sample application to demo chip features
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

#include "../../../hw_memmap.h"
#include "../../../hw_types.h"
#include "../../../src/comp.h"
#include "../../../src/debug.h"
#include "../../../src/gpio.h"
#include "../../../src/interrupt.h"
#include "../../../src/sysctl.h"
#include "../../../src/systick.h"
#include "../../../src/pwm.h"
#include "../../../src/adc.h"
#include "../../../src/uart.h"
#include "../pdc.h"

//*****************************************************************************
//
// Define I/O configuration.
//
//*****************************************************************************
#define GPIO_PIN_MUTE   GPIO_PIN_4

//*****************************************************************************
//
//! \addtogroup dk_lm3s828_list
//! <h1>DK-LM3S828 Quickstart Application (qs_dk-lm3s828)</h1>
//!
//! This example uses the potentiometer on the development board to vary the
//! rate of a click sound from the piezo buzzer.  Turning the knob in one
//! direction will result in slower clicks while turning it in the other
//! direction will result in faster clicks.  The potentiometer setting is
//! displayed on the LCD, and a log of the readings is output on the UART at
//! 115,200, 8-n-1.  The push button can be used to turn the clicking noise on
//! and off; when off the LCD and UART still provide the settings.
//
//*****************************************************************************

//*****************************************************************************
//
// Storage space for ADC results.
//
//*****************************************************************************
static unsigned long g_ulADCResults[8];

//*****************************************************************************
//
// The most recent setting of the potentiometer.
//
//*****************************************************************************
static unsigned long g_ulPotSetting;

//*****************************************************************************
//
// The number of SysTick counts between clicks of the piezo.
//
//*****************************************************************************
static unsigned long g_ulClickRate;

//*****************************************************************************
//
// Counts SysTick events.  Used to determine when the piezo should make a
// sound.
//
//*****************************************************************************
static unsigned long g_ulClickRateCount;

//*****************************************************************************
//
// The counter of the number of consecutive times that the push button has a
// value different than the currently debounced state.
//
//*****************************************************************************
static unsigned long g_ulDebounceCounter;

//*****************************************************************************
//
// A set of flags.  The flag bits are defined as follows:
//
//     0 -> The mute flag; when clear, the piezo will not be clicked and the
//          comparator results will only be available on the LCD and over the
//          UART.  When set, the piezo will click.
//     1 -> An indicator from the SysTick interrupt handler to the main code
//          that new comparator results are available.
//     2 -> The current debounced state of the push button.
//     3 -> An indicator that a SysTick interrupt has occurred.
//
//*****************************************************************************
#define FLAG_UNMUTE     0
#define FLAG_RESULTS    1
#define FLAG_BUTTON     2
#define FLAG_SYSTICK    3
static volatile unsigned long g_ulFlags;

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
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Indicate that a SysTick interrupt has occurred.
    //
    HWREGBITW(&g_ulFlags, FLAG_SYSTICK) = 1;

    //
    // Determine the tick rate.  Get the ADC value from the previous
    // acquisition cycle and start up the next acquisition.
    //
    ADCSequenceDataGet(ADC_BASE, 1, &g_ulADCResults[0]);
    ADCProcessorTrigger(ADC_BASE, 1);

    //
    // Compute the new rate at which to click the piezo.  Only 16 levels are
    // used.
    //
    g_ulPotSetting = g_ulADCResults[0] >> 6;
    g_ulClickRate = ((15 - g_ulPotSetting) + 3) * 5;

    //
    // If the current count is greater than the click rate then set the count
    // to the click rate so that the click will happen sooner.
    //
    if(g_ulClickRateCount > g_ulClickRate)
    {
        g_ulClickRateCount = g_ulClickRate;
    }

    //
    // Set the results flag once per cycle, when counter is 1.
    //
    if(g_ulClickRateCount == 1)
    {
        HWREGBITW(&g_ulFlags, FLAG_RESULTS) = 1;
    }

    //
    // Determine if the piezo should be muted.  See if the push button is in a
    // different state than the currently debounced state.
    //
    if((GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_MUTE) ? 1 : 0) !=
       HWREGBITW(&g_ulFlags, FLAG_BUTTON))
    {
        //
        // Increment the number of counts with the push button in a different
        // state.
        //
        g_ulDebounceCounter++;

        //
        // If four consecutive counts have had the push button in a different
        // state then the state has changed.
        //
        if(g_ulDebounceCounter == 4)
        {
            //
            // Toggle the debounced state of the push button.
            //
            HWREGBITW(&g_ulFlags, FLAG_BUTTON) ^= 1;

            //
            // If the push button was just pushed, then toggle the mute flag.
            //
            if(!HWREGBITW(&g_ulFlags, FLAG_BUTTON))
            {
                //
                // Toggle the mute flag.
                //
                HWREGBITW(&g_ulFlags, FLAG_UNMUTE) ^= 1;
            }
        }
    }
    else
    {
        //
        // Reset the debounce counter.
        //
        g_ulDebounceCounter = 0;
    }

    //
    // Decrement the click rate counter.
    //
    g_ulClickRateCount--;

    //
    // If the count is one then the piezo needs to be turned on.
    //
    if(g_ulClickRateCount == 1)
    {
        if(HWREGBITW(&g_ulFlags, FLAG_UNMUTE))
        {
            GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0);
        }
    }

    //
    // If the count is zero then the piezo needs to be turned off.
    //
    if(g_ulClickRateCount == 0)
    {
        //
        // Turn off the piezo.
        //
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0);

        //
        // Reset the count to the click rate.
        //
        g_ulClickRateCount = g_ulClickRate;
    }
}

//*****************************************************************************
//
// Quick start demo application.
//
//*****************************************************************************
int
main(void)
{
    unsigned long ulCount;
    char pcBuffer[4];

    //
    // Set the clocking to run directly from the crystal.  Get and store the
    // system clock frequency.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_6MHZ);

    //
    // Enable the peripherals used by this example.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Configure SysTick for a 100Hz interrupt.
    //
    SysTickPeriodSet(SysCtlClockGet() / 100);
    SysTickEnable();
    SysTickIntEnable();

    //
    // Set A0 and A1 as peripheral function for the UART.  This is used to
    // output a data log of the captured samples.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Set GPIO B0 as an output.  This drives the buzzer on the board.
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0);

    //
    // Set GPIO B4 as an input.  It is connected to the push button on the
    // board.
    //
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_4);

    //
    // ADC1 is used for potentiometer input.  Configure it to use sequence 1,
    // take one sample and stop.
    //
    ADCSequenceConfigure(ADC_BASE, 1, ADC_TRIGGER_PROCESSOR, 1);
    ADCSequenceStepConfigure(ADC_BASE, 1, 0, ADC_CTL_CH1 | ADC_CTL_END);
    ADCSequenceEnable(ADC_BASE, 1);

    //
    // Trigger the initial acquisition, so that a result will be available.
    //
    ADCProcessorTrigger(ADC_BASE, 1);

    //
    // Configure the UART.
    //
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    //
    // Initialize the peripheral device controller/LCD.
    //
    PDCInit();
    PDCLCDInit();
    PDCLCDBacklightOn();

    //
    // Create new character glyphs for the LMI logo.  The glyphs are as
    // follows:
    //
    // ...XX ..XXX XXX.. .....
    // ...XX ..XXX XXXXX .....
    // ...XX ..XX. .XXXX .....
    // ...XX ..XX. .XX.. X....
    // ...XX ..XX. .XX.. X....
    // ...XX ..XX. .XX.. XX...
    // ...XX ..XX. .XX.. XX...
    // ...XX ..XX. .XX.. XX...
    //
    // ...XX ..XX. .XX.. XX...
    // ...XX ..XX. .XX.. XX...
    // ...XX ...X. .XX.. XX...
    // ....X ...X. .XX.. XX...
    // ....X ..... ..... XX...
    // ..... XX... ..... XX...
    // ..... XXXXX XXXXX XX...
    // ..... ..XXX XXXXX XX...
    //
    PDCLCDCreateChar(0, (unsigned char *)"\003\003\003\003\003\003\003\003");
    PDCLCDCreateChar(1, (unsigned char *)"\007\007\006\006\006\006\006\006");
    PDCLCDCreateChar(2, (unsigned char *)"\034\037\017\014\014\014\014\014");
    PDCLCDCreateChar(3, (unsigned char *)"\000\000\000\020\020\030\030\030");
    PDCLCDCreateChar(4, (unsigned char *)"\003\003\003\001\001\000\000\000");
    PDCLCDCreateChar(5, (unsigned char *)"\006\006\002\002\000\030\037\007");
    PDCLCDCreateChar(6, (unsigned char *)"\014\014\014\014\000\000\037\037");
    PDCLCDCreateChar(7, (unsigned char *)"\030\030\030\030\030\030\030\030");

    //
    // Write the splash screen to the LCD.
    //
    PDCLCDSetPos(0, 0);
    PDCLCDWrite(" \000\001\002\003 Luminary", 14);
    PDCLCDSetPos(0, 1);
    PDCLCDWrite(" \004\005\006\007 Micro", 11);

    //
    // Delay for five seconds while the splash screen is displayed.
    //
    for(ulCount = 0; ulCount < (100 * 5); ulCount++)
    {
        //
        // Wait until a SysTick interrupt has occurred.
        //
        while(!HWREGBITW(&g_ulFlags, FLAG_SYSTICK))
        {
        }

        //
        // Clear the SysTick interrupt flag.
        //
        HWREGBITW(&g_ulFlags, FLAG_SYSTICK) = 0;
    }

    //
    // Clear the screen and write the main display.
    //
    PDCLCDClear();
    PDCLCDSetPos(0, 0);
    PDCLCDWrite("Turn the Pot -->", 16);
    PDCLCDSetPos(0, 1);
    PDCLCDWrite("Value:", 6);

    //
    // Set the global variables to their initial state.  The click rate
    // defaults to the slowest rate and the piezo is not muted.
    //
    g_ulPotSetting = 0;
    g_ulClickRate = 48;
    g_ulClickRateCount = 48;
    HWREGBITW(&g_ulFlags, FLAG_UNMUTE) = 1;

    //
    // Loop forever while the LED tracks the comparator value.
    //
    while(1)
    {
        //
        // See if there is a new potentiometer result.
        //
        if(HWREGBITW(&g_ulFlags, FLAG_RESULTS))
        {
            //
            // Put the new pot value into the string for the LCD.
            //
            if(g_ulPotSetting > 9)
            {
                pcBuffer[0] = '1';
            }
            else
            {
                pcBuffer[0] = ' ';
            }
            pcBuffer[1] = '0' + (g_ulPotSetting % 10);

            //
            // Write the new pot value to the UART.
            //
            UARTCharPut(UART0_BASE, pcBuffer[0]);
            UARTCharPut(UART0_BASE, pcBuffer[1]);
            UARTCharPut(UART0_BASE, '\r');
            UARTCharPut(UART0_BASE, '\n');

            //
            // Write the string to the LCD.
            //
            PDCLCDSetPos(7, 1);
            PDCLCDWrite(pcBuffer, 2);

            //
            // Clear the flag indicating a new result.
            //
            HWREGBITW(&g_ulFlags, 1) = 0;
        }
    }
}
