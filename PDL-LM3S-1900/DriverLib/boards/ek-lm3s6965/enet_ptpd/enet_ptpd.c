//*****************************************************************************
//
// enet_ptpd.c - Sample PTPd Application using lwIP.
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
#include "../../../hw_ints.h"
#include "../../../src/ethernet.h"
#include "../../../src/interrupt.h"
#include "../../../src/sysctl.h"
#include "../../../src/systick.h"
#include "../../../src/flash.h"
#include "../../../src/gpio.h"
#include "../../../utils/diag.h"
#include "../../../utils/ustdlib.h"
#include "../osram128x64x4.h"
#include "random.h"
#include "ptpd.h"
#include "globals.h"

//*****************************************************************************
//
//! \addtogroup ek_lm3s6965_list
//! <h1>Ethernet IEEE 1588 (PTPd) with lwIP (enet_ptpd)</h1>
//!
//! This example application demonstrates the operation of the Stellaris
//! Ethernet controller using the lwIP TCP/IP Stack.  DHCP is used to obtain an
//! Ethernet address.  If DHCP times out without obtaining an address, a static
//! IP address will be used.  The DHCP timeout and the default static IP are
//! easily configurable using macros.  The address that is selected will be
//! shown on the OLED display.
//!
//! A default set of pages will be served up by an internal file system and
//! the httpd server.
//!
//! The IEEE 1588 (PTP) software has been enabled in this code to synchronize
//! the internal clock to a network master clock source.
//!
//! For additional details on lwIP, refer to the lwIP web page at:
//! http://www.sics.se/~adam/lwip/
//!
//! For additional details on the PTPd software, refer to the PTPd web page at:
//! http://ptpd.sourceforge.net
//
//*****************************************************************************

//*****************************************************************************
//
// Define the system clock rate here.  One of the following must be defined to
// choose the system clock rate.
//
//*****************************************************************************
//#define SYSTEM_CLOCK_8MHZ
//#define SYSTEM_CLOCK_20MHZ
//#define SYSTEM_CLOCK_25MHZ
#define SYSTEM_CLOCK_50MHZ

//*****************************************************************************
//
// Clock and PWM dividers used depend on which system clock rate is chosen.
//
//*****************************************************************************
#if defined(SYSTEM_CLOCK_8MHZ)
#define SYSDIV      SYSCTL_SYSDIV_1
#define PWMDIV      SYSCTL_PWMDIV_1
#define CLKUSE      SYSCTL_USE_OSC
#define TICKNS      125

#elif defined(SYSTEM_CLOCK_20MHZ)
#define SYSDIV      SYSCTL_SYSDIV_10
#define PWMDIV      SYSCTL_PWMDIV_2
#define CLKUSE      SYSCTL_USE_PLL
#define TICKNS      50

#elif defined(SYSTEM_CLOCK_25MHZ)
#define SYSDIV      SYSCTL_SYSDIV_8
#define PWMDIV      SYSCTL_PWMDIV_2
#define CLKUSE      SYSCTL_USE_PLL
#define TICKNS      40

#elif defined(SYSTEM_CLOCK_50MHZ)
#define SYSDIV      SYSCTL_SYSDIV_4
#define PWMDIV      SYSCTL_PWMDIV_2
#define CLKUSE      SYSCTL_USE_PLL
#define TICKNS      20

#else
#error "System clock speed is not defined properly!"

#endif

//*****************************************************************************
//
// Pulse Per Second (PPS) Output Definitions
//
//*****************************************************************************
#define PPS_GPIO_PERIPHERAL     SYSCTL_PERIPH_GPIOB
#define PPS_GPIO_BASE           GPIO_PORTB_BASE
#define PPS_GPIO_PIN            GPIO_PIN_0

//*****************************************************************************
//
// The clock rate for the SysTick interrupt.  All events in the application
// occur at some fraction of this clock rate.
//
//*****************************************************************************
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)
#define SYSTICKUS               (1000000 / SYSTICKHZ)
#define SYSTICKNS               (1000000000 / SYSTICKHZ)

//*****************************************************************************
//
// Global Data (details in globals.h).
//
//*****************************************************************************
volatile unsigned long g_ulFlags;

//*****************************************************************************
//
// System Time - Internal representaion.
//
//*****************************************************************************
volatile unsigned long g_ulSystemTimeSeconds;
volatile unsigned long g_ulSystemTimeNanoSeconds;

//*****************************************************************************
//
// System Run Time - Ticks
//
//*****************************************************************************
volatile unsigned long g_ulSystemTimeTicks;

//*****************************************************************************
//
// Local data for clocks and timers.
//
//*****************************************************************************
static volatile unsigned long ulNewSystemTickReload = 0;
static volatile unsigned long ulSystemTickHigh = 0;
static volatile unsigned long ulSystemTickReload = 0;

//*****************************************************************************
//
// Statically allocated runtime options and parameters for PTPd.
//
//*****************************************************************************
static PtpClock PTPClock;
static ForeignMasterRecord tForeignMasterRecord[DEFUALT_MAX_FOREIGN_RECORDS];
static RunTimeOpts tRtOpts;

//*****************************************************************************
//
// External references.
//
//*****************************************************************************
extern void httpd_init(void);
extern void fs_init(void);
extern void lwip_init(void);
extern void lwip_tick(unsigned long ulTickMS);

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
// Display Date and Time.
//
//*****************************************************************************
static char g_pucBuf[23];
const char *g_ppcDay[7] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
const char *g_ppcMonth[12] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static void
DisplayDate(unsigned long ulSeconds, unsigned long ulRow)
{
    tTime sLocalTime;

    //
    // Convert the elapsed seconds (ulSeconds) into time structure.
    //
    ulocaltime(ulSeconds, &sLocalTime);

    //
    // Generate an appropriate date string for OLED display.
    //
    usprintf(g_pucBuf, "%3s %3s %2d, %4d", g_ppcDay[sLocalTime.ucWday],
             g_ppcMonth[sLocalTime.ucMon], sLocalTime.ucMday,
             sLocalTime.usYear);
    OSRAM128x64x4StringDraw(g_pucBuf, 12, ulRow, 15);
}

static void
DisplayTime(unsigned long ulSeconds, unsigned long ulRow)
{
    tTime sLocalTime;

    //
    // Convert the elapsed seconds (ulSeconds) into time structure.
    //
    ulocaltime(ulSeconds, &sLocalTime);

    //
    // Generate an appropriate date string for OLED display.
    //
    usprintf(g_pucBuf, "%02d:%02d:%02d (GMT)", sLocalTime.ucHour,
             sLocalTime.ucMin, sLocalTime.ucSec);
    OSRAM128x64x4StringDraw(g_pucBuf, 12, ulRow, 15);
}

//*****************************************************************************
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Update internal time and set PPS output, if needed.
    //
    g_ulSystemTimeNanoSeconds += SYSTICKNS;
    if(g_ulSystemTimeNanoSeconds >= 1000000000)
    {
        GPIOPinWrite(PPS_GPIO_BASE, PPS_GPIO_PIN, PPS_GPIO_PIN);
        g_ulSystemTimeNanoSeconds -= 1000000000;
        g_ulSystemTimeSeconds += 1;
        HWREGBITW(&g_ulFlags, FLAG_PPSOUT) = 1;
    }

    //
    // Set a new System Tick Reload Value.
    //
    if(ulSystemTickReload != ulNewSystemTickReload)
    {
        ulSystemTickReload = ulNewSystemTickReload ;
        g_ulSystemTimeNanoSeconds = ((g_ulSystemTimeNanoSeconds / SYSTICKNS) *
                                     SYSTICKNS);
    }

    //
    // For each tick, set the next reload value for fine tuning the clock.
    //
    if((g_ulSystemTimeTicks % TICKNS) < ulSystemTickHigh)
    {
        SysTickPeriodSet(ulSystemTickReload + 1);
    }
    else
    {
        SysTickPeriodSet(ulSystemTickReload);
    }

    //
    // Service the PTPd Timer.
    //
    timerTick(SYSTICKMS);

    //
    // Increment the run-time tick counter.
    //
    g_ulSystemTimeTicks++;

    //
    // Indicate that a SysTick interrupt has occurred.
    //
    HWREGBITW(&g_ulFlags, FLAG_SYSTICK) = 1;
}

//*****************************************************************************
//
// Initialization code for PTPD software system tick timer.
//
//*****************************************************************************
void
ptpd_systick_init(void)
{
    //
    // Initialize the System Tick Timer to run at specified frequency.
    //
    SysTickPeriodSet(SysCtlClockGet() / SYSTICKHZ);

    //
    // Initialize the timer reload values for fine-tuning in the handler.
    //
    ulSystemTickReload = ulNewSystemTickReload = SysTickPeriodGet();

    //
    // Enable the System Tick Timer.
    //
    SysTickEnable();
    SysTickIntEnable();
}

//*****************************************************************************
//
// Initialization code for PTPD software.
//
//*****************************************************************************
void
ptpd_init(void)
{
    unsigned long ulTemp;

    //
    // Clear out all of the run time options and protocol stack options.
    //
    memset(&tRtOpts, 0, sizeof(tRtOpts));
    memset(&PTPClock, 0, sizeof(PTPClock));

    //
    // Initialize all PTPd run time options to a valid, default value.
    //
    tRtOpts.syncInterval = DEFUALT_SYNC_INTERVAL;
    memcpy(tRtOpts.subdomainName, DEFAULT_PTP_DOMAIN_NAME,
           PTP_SUBDOMAIN_NAME_LENGTH);
    memcpy(tRtOpts.clockIdentifier, IDENTIFIER_DFLT, PTP_CODE_STRING_LENGTH);
    tRtOpts.clockVariance = DEFAULT_CLOCK_VARIANCE;
    tRtOpts.clockStratum = DEFAULT_CLOCK_STRATUM;
    tRtOpts.clockPreferred = FALSE;
    tRtOpts.currentUtcOffset = DEFAULT_UTC_OFFSET;
    tRtOpts.epochNumber = 0;
    memcpy(tRtOpts.ifaceName, "LMI", strlen("LMI"));
    tRtOpts.noResetClock = DEFAULT_NO_RESET_CLOCK;
    tRtOpts.noAdjust = FALSE;
    tRtOpts.displayStats = FALSE;
    tRtOpts.csvStats = FALSE;
    tRtOpts.unicastAddress[0] = 0;
    tRtOpts.ap = DEFAULT_AP;
    tRtOpts.ai = DEFAULT_AI;
    tRtOpts.s = DEFAULT_DELAY_S;
    tRtOpts.inboundLatency.seconds = 0;
    tRtOpts.inboundLatency.nanoseconds = DEFAULT_INBOUND_LATENCY;
    tRtOpts.outboundLatency.seconds = 0;
    tRtOpts.outboundLatency.nanoseconds = DEFAULT_OUTBOUND_LATENCY;
    tRtOpts.max_foreign_records = DEFUALT_MAX_FOREIGN_RECORDS;
    tRtOpts.slaveOnly = TRUE;
    tRtOpts.probe = FALSE;
    tRtOpts.probe_management_key = 0;
    tRtOpts.probe_record_key = 0;
    tRtOpts.halfEpoch = FALSE;

    //
    // Initialize the PTP Clock Fields.
    //
    PTPClock.foreign = &tForeignMasterRecord[0];

    //
    // Configure port "uuid" parameters.
    //
    PTPClock.port_communication_technology = PTP_ETHER;
    EthernetMACAddrGet(ETH_BASE, (unsigned char *)PTPClock.port_uuid_field);

    //
    // Enable Ethernet Multicast Reception (required for PTPd operation).
    // Note:  This must follow lwIP/Ethernet initialization.
    //
    ulTemp = EthernetConfigGet(ETH_BASE);
    ulTemp |= ETH_CFG_RX_AMULEN;
    EthernetConfigSet(ETH_BASE, ulTemp);

    //
    // Run the protocol engine for the first time to initialize the state
    // machines.
    //
    protocol_first(&tRtOpts, &PTPClock);
}

//*****************************************************************************
//
// Run the protocol engine loop/poll.
//
//*****************************************************************************
void
ptpd_tick(void)
{
    //
    // Run the protocol engine for each pass through the main process loop.
    //
    protocol_loop(&tRtOpts, &PTPClock);
}

//*****************************************************************************
//
// Main entry point for sample PTPd/lwIP application.
//
//*****************************************************************************
int
main(void)
{
    unsigned long ulUser0, ulUser1;
    unsigned char pucMACArray[8];

    //
    // Set the clocking to run directly from the crystal (see globals.h for
    // definitions of SYSDIV and CLKUSE).
    //
    SysCtlClockSet(SYSDIV | CLKUSE | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);

    //
    // Initialize the OLED display.
    //
    OSRAM128x64x4Init(1000000);
    OSRAM128x64x4StringDraw("PTPd with lwIP", 24, 0, 15);

    //
    // Check for presence of Ethernet Controller.
    //
    if(!SysCtlPeripheralPresent(SYSCTL_PERIPH_ETH))
    {
        OSRAM128x64x4StringDraw("Ethernet Controller", 0, 16, 15);
        OSRAM128x64x4StringDraw("Not Found!", 0, 24, 15);
        DiagExit(1);
    }

    //
    // Enable and Reset the Ethernet Controller.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
    SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);

    //
    // Enable Port F for Ethernet LEDs.
    //  LED0        Bit 3   Output
    //  LED1        Bit 2   Output
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, GPIO_DIR_MODE_HW);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3,
                     GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);

    //
    // Configure the defined PPS GPIO for output.
    //
    SysCtlPeripheralEnable(PPS_GPIO_PERIPHERAL);
    GPIOPinTypeGPIOOutput(PPS_GPIO_BASE, PPS_GPIO_PIN);
    GPIOPinWrite(PPS_GPIO_BASE, PPS_GPIO_PIN, 0);

    //
    // Configure SysTick for a periodic interrupt in PTPd system.
    //
    ptpd_systick_init();

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.
    //
    // For the LMI Evaluation Kits, the MAC address will be stored in the
    // non-volatile USER0 and USER1 registers.  These registers can be read
    // using the FlashUserGet function, as illustrated below.
    //
    FlashUserGet(&ulUser0, &ulUser1);
    if((ulUser0 == 0xffffffff) || (ulUser1 == 0xffffffff))
    {
        //
        // We should never get here.  This is an error if the MAC address has
        // not been programmed into the device.  Exit the program.
        //
        OSRAM128x64x4StringDraw("MAC Address", 0, 16, 15);
        OSRAM128x64x4StringDraw("Not Programmed!", 0, 24, 15);
        DiagExit(2);
    }

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    //
    pucMACArray[0] = ((ulUser0 >> 0) & 0xff);
    pucMACArray[1] = ((ulUser0 >> 8) & 0xff);
    pucMACArray[2] = ((ulUser0 >> 16) & 0xff);
    pucMACArray[3] = ((ulUser1 >> 0) & 0xff);
    pucMACArray[4] = ((ulUser1 >> 8) & 0xff);
    pucMACArray[5] = ((ulUser1 >> 16) & 0xff);

    //
    // Program the hardware with it's MAC address (for filtering).
    //
    EthernetMACAddrSet(ETH_BASE, pucMACArray);

    //
    // Initialize the file system.
    //
    fs_init();

    //
    // Initialize the Random Number Generator.
    //
    RandomSeed();

    //
    // Initialize all of the lwIP code, as needed, which will also initialize
    // the low-level Ethernet code.
    //
    lwip_init();

    //
    // Initialize a sample web server application.
    //
    httpd_init();

    //
    // Main Application Loop (for systems with no RTOS).  Run every SYSTICK.
    //
    while(true)
    {
        //
        // Wait for an event to occur.
        //
        while(!g_ulFlags)
        {
        }

        //
        // Check if a SYSTICK has occurred.
        //
        if(HWREGBITW(&g_ulFlags, FLAG_SYSTICK))
        {
            //
            // Clear the SysTick interrupt flag.
            //
            HWREGBITW(&g_ulFlags, FLAG_SYSTICK) = 0;

            //
            // Run the Luminary lwIP system tick.
            //
            lwip_tick(SYSTICKMS);

            //
            // Clear PPS output when needed and display time of day.
            //
            if(HWREGBITW(&g_ulFlags, FLAG_PPSOFF))
            {
                //
                // Negate the PPS output.
                //
                GPIOPinWrite(PPS_GPIO_BASE, PPS_GPIO_PIN, 0);

                //
                // Indicate that we have negated the PPS output.
                //
                HWREGBITW(&g_ulFlags, FLAG_PPSOFF) = 0;

                //
                // Display Date and Time.
                //
                DisplayDate(g_ulSystemTimeSeconds, 48);
                DisplayTime(g_ulSystemTimeSeconds, 56);
            }

            //
            // Setup to disable the PPS output on the next pass.
            //
            if(HWREGBITW(&g_ulFlags, FLAG_PPSOUT))
            {
                //
                // Setup to turn off the PPS output.
                //
                HWREGBITW(&g_ulFlags, FLAG_PPSOUT) = 0;
                HWREGBITW(&g_ulFlags, FLAG_PPSOFF) = 1;
            }
        }

        //
        // Check if an RX Packet was received.
        //
        if(HWREGBITW(&g_ulFlags, FLAG_RXPKT))
        {
            //
            // Clear the Rx Packet interrupt flag.
            //
            HWREGBITW(&g_ulFlags, FLAG_RXPKT) = 0;

            //
            // Run the Luminary lwIP system tick, but with no time, to indicate
            // an RX or TX packet has occurred.
            //
            lwip_tick(0);

            //
            // Enable Ethernet RX Packet Interrupts.
            //
            EthernetIntEnable(ETH_BASE, ETH_INT_RX);
        }

        //
        // Check if a TX Packet was received.
        //
        if(HWREGBITW(&g_ulFlags, FLAG_TXPKT))
        {
            //
            // Clear the Rx Packet interrupt flag.
            //
            HWREGBITW(&g_ulFlags, FLAG_TXPKT) = 0;

            //
            // Run the Luminary lwIP system tick, but with no time, to indicate
            // an RX or TX packet has occurred.
            //
            lwip_tick(0);

            //
            // Enable Ethernet RX Packet Interrupts.
            //
            EthernetIntEnable(ETH_BASE, ETH_INT_TX);
        }

        //
        // If IP address has been assigned, initialize the PTPD software (if
        // not already initialized).
        //
        if( HWREGBITW(&g_ulFlags, FLAG_IPADDR) &&
           !HWREGBITW(&g_ulFlags, FLAG_PTPDINIT))
        {
            ptpd_init();
            HWREGBITW(&g_ulFlags, FLAG_PTPDINIT) = 1;
        }

        //
        // If PTPD software has been initialized, run the ptpd tick.
        //
        if(HWREGBITW(&g_ulFlags, FLAG_PTPDINIT))
        {
            ptpd_tick();
        }
    }
}

//*****************************************************************************
//
// The following set of functions are LMI Board/Chip Specific implementations
// of functions required by PTPd software.  Prototypes are defined in ptpd.h,
// or one of its included files.
//
//*****************************************************************************

//*****************************************************************************
//
// Display Statistics.  For now, do nothing, but this could be used to either
// update a web page, send data to the serial port, or to the OLED display.
//
// Refer to the ptpd software "src/dep/sys.c" for example code.
//
//*****************************************************************************
void
displayStats(RunTimeOpts *rtOpts, PtpClock *ptpClock)
{
}

//*****************************************************************************
//
// This function returns the local time (in PTPd internal time format).  This
// time is maintained by the SysTick interupt.
//
//*****************************************************************************
void
getTime(TimeInternal *time)
{
    //
    // Fill in the seconds field from the System Tick Handler value.
    //
    time->seconds = g_ulSystemTimeSeconds;

    //
    // Fill in the nanoseconds field from the System Tick Handler value,
    // adjusted for elapsed time from the most recent System Tick interrupt.
    //
    time->nanoseconds = (g_ulSystemTimeNanoSeconds +
                         ((SysTickPeriodGet() - SysTickValueGet()) * TICKNS));
}

//*****************************************************************************
//
// This function will set the local time (provided in PTPd internal time
// format).  This time is maintained by the SysTick interrupt.
//
//*****************************************************************************
void
setTime(TimeInternal *time)
{
    //
    // Update the System Tick Handler time values from the given PTPd time
    // (fine-tuning is handled in the System Tick handler).
    //
    g_ulSystemTimeSeconds = time->seconds;
    g_ulSystemTimeNanoSeconds = time->nanoseconds;
}

//*****************************************************************************
//
// This function returns a random number, using the functions in random.c.
//
//*****************************************************************************
UInteger16
getRand(UInteger32 *seed)
{
    unsigned long ulTemp;
    UInteger16 uiTemp;

    //
    // Re-seed the random number generator.
    //
    RandomAddEntropy(*seed);
    RandomSeed();

    //
    // Get a random number and return a 16-bit, truncated version.
    //
    ulTemp = RandomNumber();
    uiTemp = (UInteger16)(ulTemp & 0xFFFF);
    return(uiTemp);
}

//*****************************************************************************
//
// Based on the value (adj) provided by the PTPd Clock Servo routine, this
// function will adjust the SysTick periodic interval to allow fine-tuning of
// the PTP Clock.
//
//*****************************************************************************
Boolean
adjFreq(Integer32 adj)
{
    unsigned long ulTemp;

    //
    // Check for max/min value of adjustment.
    //
    if(adj > ADJ_MAX)
    {
        adj = ADJ_MAX;
    }
    else if(adj < -ADJ_MAX)
    {
        adj = -ADJ_MAX;
    }

    //
    // Convert input to nanoseconds / systick.
    //
    adj = adj / SYSTICKHZ;

    //
    // Get the nominal tick reload value and convert to nano seconds.
    //
    ulTemp = (SysCtlClockGet() / SYSTICKHZ) * TICKNS;

    //
    // Factor in the adjustment.
    //
    ulTemp -= adj;

    //
    // Get a modulo count of nanoseconds for fine tuning.
    //
    ulSystemTickHigh = ulTemp % TICKNS;

    //
    // Set the reload value.
    //
    ulNewSystemTickReload = ulTemp / TICKNS;

    //
    // Return.
    //
    return(TRUE);
}
