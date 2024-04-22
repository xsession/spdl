//*****************************************************************************
//
// qs_ek-lm3s8962.c - The quick start application for the LM3S8962 Evaluation
//                    Board.
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
#include "../../../hw_sysctl.h"
#include "../../../hw_types.h"
#include "../../../src/adc.h"
#include "../../../src/can.h"
#include "../../../src/debug.h"
#include "../../../src/gpio.h"
#include "../../../src/interrupt.h"
#include "../../../src/pwm.h"
#include "../../../src/sysctl.h"
#include "../../../src/systick.h"
#include "../../../src/timer.h"
#include "../../../src/uart.h"
#include "../rit128x96x4.h"
#include "audio.h"
#include "can_net.h"
#include "enet.h"
#include "game.h"
#include "globals.h"
#include "images.h"
#include "random.h"
#include "screen_saver.h"
#include "sounds.h"

//*****************************************************************************
//
//! \addtogroup ek_lm3s8962_list
//! <h1>EK-LM3S8962 Quickstart Application (qs_ek-lm3s8962)</h1>
//!
//! A game in which a blob-like character tries to find its way out of a maze.
//! The character starts in the middle of the maze and must find the exit,
//! which will always be located at one of the four corners of the maze.  Once
//! the exit to the maze is located, the character is placed into the middle of
//! a new maze and must find the exit to that maze; this repeats endlessly.
//!
//! The game is started by pressing the select push button on the right side
//! of the board.  During game play, the select push button will fire a bullet
//! in the direction the character is currently facing, and the navigation push
//! buttons on the left side of the board will cause the character to walk in
//! the corresponding direction.
//!
//! Populating the maze are a hundred spinning stars that mindlessly attack the
//! character.  Contact with one of these stars results in the game ending, but
//! the stars go away when shot.
//!
//! Score is accumulated for shooting the stars and for finding the exit to the
//! maze.  The game lasts for only one character, and the score is displayed on
//! the virtual UART at 115,200, 8-N-1 during game play and will be displayed
//! on the screen at the end of the game.
//!
//! A small web site is provided by the game over the Ethernet port.  DHCP is
//! used to obtain an Ethernet address.  If DHCP times out without obtaining an
//! address, a static IP address will be used.  The DHCP timeout and the
//! default static IP are easily configurable using macros.  The address that
//! is selected will be shown on the OLED display before the game starts.  The
//! web pages allow the entire game maze to be viewed, along with the character
//! and stars; the display is generated by a Java applet that is downloaded
//! from the game (therefore requiring that Java be installed in the web
//! browser).  The volume of the game music and sound effects can also be
//! adjusted.
//!
//! If the CAN device board is attached and is running the can_device_qs
//! application, the volume of the music and sound effects can be adjusted over
//! CAN with the two push buttons on the target board.  The LED on the CAN
//! device board will track the state of the LED on the main board via CAN
//! messages.  The operation of the game will not be affected by the absence of
//! the CAN device board.
//!
//! Since the OLED display on the evaluation board has burn-in characteristics
//! similar to a CRT, the application also contains a screen saver.  The screen
//! saver will only become active if two minutes have passed without the user
//! push button being pressed while waiting to start the game (that is, it will
//! never come on during game play).  Qix-style bouncing lines are drawn on the
//! display by the screen saver.
//!
//! After two minutes of running the screen saver, the display will be turned
//! off and the user LED will blink.  Either mode of screen saver (bouncing
//! lines or blank display) will be exited by pressing the select push button.
//! The select push button will then need to be pressed again to start the
//! game.
//
//*****************************************************************************

//*****************************************************************************
//
// A set of flags used to track the state of the application.
//
//*****************************************************************************
unsigned long g_ulFlags;

//*****************************************************************************
//
// The speed of the processor clock, which is therefore the speed of the clock
// that is fed to the peripherals.
//
//*****************************************************************************
unsigned long g_ulSystemClock;

//*****************************************************************************
//
// Storage for a local frame buffer.
//
//*****************************************************************************
unsigned char g_pucFrame[6144];

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
// The number of clock ticks that have occurred.  This is used as an entropy
// source for the random number generator; the number of clock ticks at the
// time of a button press or release is an entropic event.
//
//*****************************************************************************
static unsigned long g_ulTickCount = 0;

//*****************************************************************************
//
// The number of clock ticks that have occurred since the last screen update
// was requested.  This is used to divide down the system clock tick to the
// desired screen update rate.
//
//*****************************************************************************
static unsigned char g_ucScreenUpdateCount = 0;

//*****************************************************************************
//
// The number of clock ticks that have occurred since the last application
// update was performed.  This is used to divide down the system clock tick to
// the desired application update rate.
//
//*****************************************************************************
static unsigned char g_ucAppUpdateCount = 0;

//*****************************************************************************
//
// The debounced state of the five push buttons.  The bit positions correspond
// to:
//
//     0 - Up
//     1 - Down
//     2 - Left
//     3 - Right
//     4 - Select
//
//*****************************************************************************
unsigned char g_ucSwitches = 0x1f;

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
// Handles the SysTick timeout interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    unsigned long ulData, ulDelta;

    //
    // Increment the tick count.
    //
    g_ulTickCount++;

    //
    // Indicate that a timer interrupt has occurred.
    //
    HWREGBITW(&g_ulFlags, FLAG_CLOCK_TICK) = 1;

    //
    // Increment the screen update count.
    //
    g_ucScreenUpdateCount++;

    //
    // See if 1/30th of a second has passed since the last screen update.
    //
    if(g_ucScreenUpdateCount == (CLOCK_RATE / 30))
    {
        //
        // Restart the screen update count.
        //
        g_ucScreenUpdateCount = 0;

        //
        // Request a screen update.
        //
        HWREGBITW(&g_ulFlags, FLAG_UPDATE) = 1;
    }

    //
    // Update the music/sound effects.
    //
    AudioHandler();

    //
    // Increment the application update count.
    //
    g_ucAppUpdateCount++;

    //
    // See if 1/100th of a second has passed since the last application update.
    //
    if(g_ucAppUpdateCount != (CLOCK_RATE / 100))
    {
        //
        // Return without doing any further processing.
        //
        return;
    }

    //
    // Restart the application update count.
    //
    g_ucAppUpdateCount = 0;

    //
    // Run the Ethernet handler.
    //
    EnetTick(10);

    //
    // Read the state of the push buttons.
    //
    ulData = (GPIOPinRead(GPIO_PORTE_BASE, (GPIO_PIN_0 | GPIO_PIN_1 |
                                            GPIO_PIN_2 | GPIO_PIN_3)) |
              (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) << 3));

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
    // See if any switches just changed debounced state.
    //
    if(ulDelta)
    {
        //
        // Add the current tick count to the entropy pool.
        //
        RandomAddEntropy(g_ulTickCount);
    }

    //
    // See if the select button was just pressed.
    //
    if((ulDelta & 0x10) && !(g_ucSwitches & 0x10))
    {
        //
        // Set a flag to indicate that the select button was just pressed.
        //
        HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 1;
    }
}

//*****************************************************************************
//
// Delay for a multiple of the system tick clock rate.
//
//*****************************************************************************
static void
Delay(unsigned long ulCount)
{
    //
    // Loop while there are more clock ticks to wait for.
    //
    while(ulCount--)
    {
        //
        // Wait until a SysTick interrupt has occurred.
        //
        while(!HWREGBITW(&g_ulFlags, FLAG_CLOCK_TICK))
        {
        }

        //
        // Clear the SysTick interrupt flag.
        //
        HWREGBITW(&g_ulFlags, FLAG_CLOCK_TICK) = 0;
    }
}

//*****************************************************************************
//
// Displays a logo for a specified amount of time.
//
//*****************************************************************************
static void
DisplayLogo(const unsigned char *pucLogo, unsigned long ulWidth,
            unsigned long ulHeight, unsigned long ulDelay)
{
    unsigned char *pucDest, ucHigh, ucLow;
    unsigned long ulLoop1, ulLoop2;
    const unsigned char *pucSrc;
    long lIdx;

    //
    // Loop through thirty two intensity levels to fade the logo in from black.
    //
    for(lIdx = 1; lIdx <= 32; lIdx++)
    {
        //
        // Clear the local frame buffer.
        //
        for(ulLoop1 = 0; ulLoop1 < sizeof(g_pucFrame); ulLoop1 += 4)
        {
            *(unsigned long *)(g_pucFrame + ulLoop1) = 0;
        }

        //
        // Get a pointer to the beginning of the logo.
        //
        pucSrc = pucLogo;

        //
        // Get a point to the upper left corner of the frame buffer where the
        // logo will be placed.
        //
        pucDest = (g_pucFrame + (((96 - ulHeight) / 2) * 64) +
                   ((128 - ulWidth) / 4));

        //
        // Copy the logo into the frame buffer, scaling the intensity.  Loop
        // over the rows.
        //
        for(ulLoop1 = 0; ulLoop1 < ulHeight; ulLoop1++)
        {
            //
            // Loop over the columns.
            //
            for(ulLoop2 = 0; ulLoop2 < (ulWidth / 2); ulLoop2++)
            {
                //
                // Get the two nibbles of the next byte from the source.
                //
                ucHigh = pucSrc[ulLoop2] >> 4;
                ucLow = pucSrc[ulLoop2] & 15;

                //
                // Scale the intensity of the two nibbles.
                //
                ucHigh = ((unsigned long)ucHigh * lIdx) / 32;
                ucLow = ((unsigned long)ucLow * lIdx) / 32;

                //
                // Write the two nibbles to the frame buffer.
                //
                pucDest[ulLoop2] = (ucHigh << 4) | ucLow;
            }

            //
            // Increment to the next row of the source and destination.
            //
            pucSrc += (ulWidth / 2);
            pucDest += 64;
        }

        //
        // Wait until an update has been requested.
        //
        while(HWREGBITW(&g_ulFlags, FLAG_UPDATE) == 0)
        {
        }

        //
        // Clear the update request flag.
        //
        HWREGBITW(&g_ulFlags, FLAG_UPDATE) = 0;

        //
        // Display the local frame buffer on the display.
        //
        RIT128x96x4ImageDraw(g_pucFrame, 0, 0, 128, 96);
    }

    //
    // Delay for the specified time while the logo is displayed.
    //
    Delay(ulDelay);

    //
    // Loop through the thirty two intensity levels to face the logo back to
    // black.
    //
    for(lIdx = 31; lIdx >= 0; lIdx--)
    {
        //
        // Clear the local frame buffer.
        //
        for(ulLoop1 = 0; ulLoop1 < sizeof(g_pucFrame); ulLoop1 += 4)
        {
            *(unsigned long *)(g_pucFrame + ulLoop1) = 0;
        }

        //
        // Get a pointer to the beginning of the logo.
        //
        pucSrc = pucLogo;

        //
        // Get a point to the upper left corner of the frame buffer where the
        // logo will be placed.
        //
        pucDest = (g_pucFrame + (((96 - ulHeight) / 2) * 64) +
                   ((128 - ulWidth) / 4));

        //
        // Copy the logo into the frame buffer, scaling the intensity.  Loop
        // over the rows.
        //
        for(ulLoop1 = 0; ulLoop1 < ulHeight; ulLoop1++)
        {
            //
            // Loop over the columns.
            //
            for(ulLoop2 = 0; ulLoop2 < (ulWidth / 2); ulLoop2++)
            {
                //
                // Get the two nibbles of the next byte from the source.
                //
                ucHigh = pucSrc[ulLoop2] >> 4;
                ucLow = pucSrc[ulLoop2] & 15;

                //
                // Scale the intensity of the two nibbles.
                //
                ucHigh = ((unsigned long)ucHigh * lIdx) / 32;
                ucLow = ((unsigned long)ucLow * lIdx) / 32;

                //
                // Write the two nibbles to the frame buffer.
                //
                pucDest[ulLoop2] = (ucHigh << 4) | ucLow;
            }

            //
            // Increment to the next row of the source and destination.
            //
            pucSrc += (ulWidth / 2);
            pucDest += 64;
        }

        //
        // Wait until an update has been requested.
        //
        while(HWREGBITW(&g_ulFlags, FLAG_UPDATE) == 0)
        {
        }

        //
        // Clear the update request flag.
        //
        HWREGBITW(&g_ulFlags, FLAG_UPDATE) = 0;

        //
        // Display the local frame buffer on the display.
        //
        RIT128x96x4ImageDraw(g_pucFrame, 0, 0, 128, 96);
    }
}

//*****************************************************************************
//
// The main code for the application.  It sets up the peripherals, displays the
// splash screens, and then manages the interaction between the game and the
// screen saver.
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
    // Set the clocking to run at 50MHz from the PLL.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_8);

    //
    // Get the system clock speed.
    //
    g_ulSystemClock = SysCtlClockGet();

    //
    // Enable the peripherals used by the application.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure the GPIOs used to read the state of the on-board push buttons.
    //
    GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,
                         GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);
    GPIOPadConfigSet(GPIO_PORTE_BASE,
                     GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA,
                     GPIO_PIN_TYPE_STD_WPU);

    //
    // Configure the LED, speaker, and UART GPIOs as required.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0);

    //
    // Initialize the CAN controller.
    //
    CANConfigure();

    //
    // Intialize the Ethernet Controller and TCP/IP Stack.
    //
    EnetInit();

    //
    // Configure the first UART for 115,200, 8-N-1 operation.
    //
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
    UARTEnable(UART0_BASE);

    //
    // Send a welcome message to the UART.
    //
    UARTCharPut(UART0_BASE, 'W');
    UARTCharPut(UART0_BASE, 'e');
    UARTCharPut(UART0_BASE, 'l');
    UARTCharPut(UART0_BASE, 'c');
    UARTCharPut(UART0_BASE, 'o');
    UARTCharPut(UART0_BASE, 'm');
    UARTCharPut(UART0_BASE, 'e');
    UARTCharPut(UART0_BASE, '\r');
    UARTCharPut(UART0_BASE, '\n');

    //
    // Initialize the OSRAM OLED display.
    //
    RIT128x96x4Init(3500000);

    //
    // Initialize the PWM for generating music and sound effects.
    //
    AudioOn();

    //
    // Configure SysTick to periodically interrupt.
    //
    SysTickPeriodSet(g_ulSystemClock / CLOCK_RATE);
    SysTickIntEnable();
    SysTickEnable();

    //
    // Delay for a bit to allow the initial display flash to subside.
    //
    Delay(CLOCK_RATE / 4);

    //
    // Play the intro music.
    //
    AudioPlaySong(g_pusIntro, sizeof(g_pusIntro) / 2);

    //
    // Display the Luminary Micro logo for five seconds.
    //
    DisplayLogo(g_pucLMILogo, 120, 42, 5 * CLOCK_RATE);

    //
    // Display the Keil/ARM logo for five seconds.
    //
#if defined(rvmdk) || defined(__ARMCC_VERSION)
    DisplayLogo(g_pucKeilLogo, 128, 40, 5 * CLOCK_RATE);
#endif

    //
    // Display the IAR logo for five seconds.
    //
#if defined(ewarm)
    DisplayLogo(g_pucIarLogo, 102, 61, 5 * CLOCK_RATE);
#endif

    //
    // Display the CodeSourcery logo for five seconds.
    //
#if defined(gcc) || defined(sourcerygxx)
    DisplayLogo(g_pucCodeSourceryLogo, 128, 34, 5 * CLOCK_RATE);
#endif

    //
    // Throw away any button presses that may have occurred while the splash
    // screens were being displayed.
    //
    HWREGBITW(&g_ulFlags, FLAG_BUTTON_PRESS) = 0;

    //
    // Loop forever.
    //
    while(1)
    {
        //
        // Display the main screen.
        //
        if(MainScreen())
        {
            //
            // The button was pressed, so start the game.
            //
            PlayGame();
        }
        else
        {
            //
            // The button was not pressed during the timeout period, so start
            // the screen saver.
            //
            ScreenSaver();
        }
    }
}
