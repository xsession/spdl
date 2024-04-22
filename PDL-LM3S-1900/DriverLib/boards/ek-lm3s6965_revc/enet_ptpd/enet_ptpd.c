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
#include "../../../src/timer.h"
#include "../../../utils/diag.h"
#include "../../../utils/ustdlib.h"
#include "../../../utils/uartstdio.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "../rit128x96x4.h"
#include "random.h"
#include "ptpd.h"
#include "globals.h"

//*****************************************************************************
//
//! \addtogroup ek_lm3s6965_revc_list
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
// The following group of labels define the priorities of each of the interrupt
// we use in this example.  SysTick must be high priority and capable of
// preempting other interrupts to minimize the effect of system loading on the
// timestamping mechanism.
//
// The application uses the default Priority Group setting of 0 which means
// that we have 8 possible preemptable interrupt levels available to us using
// the 3 bits of priority available on the Stellaris microcontrollers with
// values from 0xE0 (lowest priority) to 0x00 (highest priority).
//
//*****************************************************************************
#define SYSTICK_INT_PRIORITY  0x00
#define ETHERNET_INT_PRIORITY 0x80

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
// Event flags (bit positions defined in globals.h)
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
// These debug variables track the number of times the getTime function reckons
// it detected a SysTick wrap occurring during the time when it was reading the
// time.  We also record the second value of the timestamp when the wrap was
// detected in case we want to try to correlate this with any external
// measurements.
//
//*****************************************************************************
#ifdef DEBUG
unsigned long g_ulSysTickWrapDetect = 0;
unsigned long g_ulSysTickWrapTime = 0;
unsigned long g_ulGetTimeWrapCount = 0;
#endif

//*****************************************************************************
//
// Local data for clocks and timers.
//
//*****************************************************************************
static volatile unsigned long g_ulNewSystemTickReload = 0;
static volatile unsigned long g_ulSystemTickHigh = 0;
static volatile unsigned long g_ulSystemTickReload = 0;

//*****************************************************************************
//
// Statically allocated runtime options and parameters for PTPd.
//
//*****************************************************************************
static PtpClock g_sPTPClock;
static ForeignMasterRecord g_psForeignMasterRec[DEFUALT_MAX_FOREIGN_RECORDS];
static RunTimeOpts g_sRtOpts;

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
    RIT128x96x4StringDraw(g_pucBuf, 12, ulRow, 15);
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
    RIT128x96x4StringDraw(g_pucBuf, 12, ulRow, 15);
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
    if(g_ulSystemTickReload != g_ulNewSystemTickReload)
    {
        g_ulSystemTickReload = g_ulNewSystemTickReload;

        g_ulSystemTimeNanoSeconds = ((g_ulSystemTimeNanoSeconds / SYSTICKNS) *
                                     SYSTICKNS);
    }

    //
    // For each tick, set the next reload value for fine tuning the clock.
    //
    if((g_ulSystemTimeTicks % TICKNS) < g_ulSystemTickHigh)
    {
        SysTickPeriodSet(g_ulSystemTickReload + 1);
    }
    else
    {
        SysTickPeriodSet(g_ulSystemTickReload);
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
    g_ulSystemTickReload = SysTickPeriodGet();
    g_ulNewSystemTickReload = g_ulSystemTickReload;

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
    memset(&g_sRtOpts, 0, sizeof(g_sRtOpts));
    memset(&g_sPTPClock, 0, sizeof(g_sPTPClock));

    //
    // Initialize all PTPd run time options to a valid, default value.
    //
    g_sRtOpts.syncInterval = DEFUALT_SYNC_INTERVAL;
    memcpy(g_sRtOpts.subdomainName, DEFAULT_PTP_DOMAIN_NAME,
           PTP_SUBDOMAIN_NAME_LENGTH);
    memcpy(g_sRtOpts.clockIdentifier, IDENTIFIER_DFLT, PTP_CODE_STRING_LENGTH);
    g_sRtOpts.clockVariance = DEFAULT_CLOCK_VARIANCE;
    g_sRtOpts.clockStratum = DEFAULT_CLOCK_STRATUM;
    g_sRtOpts.clockPreferred = FALSE;
    g_sRtOpts.currentUtcOffset = DEFAULT_UTC_OFFSET;
    g_sRtOpts.epochNumber = 0;
    memcpy(g_sRtOpts.ifaceName, "LMI", strlen("LMI"));
    g_sRtOpts.noResetClock = DEFAULT_NO_RESET_CLOCK;
    g_sRtOpts.noAdjust = FALSE;
    g_sRtOpts.displayStats = FALSE;
    g_sRtOpts.csvStats = FALSE;
    g_sRtOpts.unicastAddress[0] = 0;
    g_sRtOpts.ap = DEFAULT_AP;
    g_sRtOpts.ai = DEFAULT_AI;
    g_sRtOpts.s = DEFAULT_DELAY_S;
    g_sRtOpts.inboundLatency.seconds = 0;
    g_sRtOpts.inboundLatency.nanoseconds = DEFAULT_INBOUND_LATENCY;
    g_sRtOpts.outboundLatency.seconds = 0;
    g_sRtOpts.outboundLatency.nanoseconds = DEFAULT_OUTBOUND_LATENCY;
    g_sRtOpts.max_foreign_records = DEFUALT_MAX_FOREIGN_RECORDS;
    g_sRtOpts.slaveOnly = TRUE;
    g_sRtOpts.probe = FALSE;
    g_sRtOpts.probe_management_key = 0;
    g_sRtOpts.probe_record_key = 0;
    g_sRtOpts.halfEpoch = FALSE;

    //
    // Initialize the PTP Clock Fields.
    //
    g_sPTPClock.foreign = &g_psForeignMasterRec[0];

    //
    // Configure port "uuid" parameters.
    //
    g_sPTPClock.port_communication_technology = PTP_ETHER;
    EthernetMACAddrGet(ETH_BASE, (unsigned char *)g_sPTPClock.port_uuid_field);

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
    protocol_first(&g_sRtOpts, &g_sPTPClock);
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
    protocol_loop(&g_sRtOpts, &g_sPTPClock);
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
    // Set the system clocking as defined above in SYSDIV and CLKUSE.
    //
    SysCtlClockSet(SYSDIV | CLKUSE | SYSCTL_OSC_MAIN | SYSCTL_XTAL_8MHZ);

    //
    // Set up for debug output to the UART.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioInit(0);

    //
    // Initialize the OLED display.
    //
    RIT128x96x4Init(1000000);
    RIT128x96x4StringDraw("PTPd with lwIP", 24, 0, 15);

    //
    // Enable and Reset the Ethernet Controller.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
    SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);
    IntPrioritySet(INT_ETH, ETHERNET_INT_PRIORITY);

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
    // For the Luminary Micro Evaluation Kits, the MAC address will be stored
    // in the non-volatile USER0 and USER1 registers.  These registers can be
    // read using the FlashUserGet function, as illustrated below.
    //
    FlashUserGet(&ulUser0, &ulUser1);
    if((ulUser0 == 0xffffffff) || (ulUser1 == 0xffffffff))
    {
        //
        // We should never get here.  This is an error if the MAC address has
        // not been programmed into the device.  Exit the program.
        //
        RIT128x96x4StringDraw("MAC Address", 0, 16, 15);
        RIT128x96x4StringDraw("Not Programmed!", 0, 24, 15);
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
        while(!(g_ulFlags & MODE_FLAG_MASK))
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
        }

        //
        // Check if a TX Packet was sent.
        //
        if(HWREGBITW(&g_ulFlags, FLAG_TXPKT))
        {
            //
            // Clear the Tx Packet interrupt flag.
            //
            HWREGBITW(&g_ulFlags, FLAG_TXPKT) = 0;

            //
            // Run the Luminary lwIP system tick, but with no time, to indicate
            // an RX or TX packet has occurred.
            //
            lwip_tick(0);

            //
            // Enable Ethernet TX Packet Interrupts.
            //
            EthernetIntEnable(ETH_BASE, ETH_INT_TX);
        }

        //
        // If IP address has been assigned, initialize the PTPD software (if
        // not already initialized).
        //
        if(HWREGBITW(&g_ulFlags, FLAG_IPADDR) &&
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
// time is maintained by the SysTick interrupt.
//
// Note: It is very important to ensure that we detect cases where the system
// tick rolls over during this function. If we don't do this, there is a race
// condition that will cause the reported time to be off by a second or so
// once in a blue moon. This, in turn, causes large perturbations in the
// 1588 time controller resulting in large deltas for many seconds as the
// controller tries to compensate.
//
//*****************************************************************************
void
getTime(TimeInternal *time)
{
    unsigned long ulTime1;
    unsigned long ulTime2;
    unsigned long ulSeconds;
    unsigned long ulPeriod;
    unsigned long ulNanoseconds;

    //
    // We read the SysTick value twice, sandwiching taking snapshots of
    // the seconds, nanoseconds and period values. If the second SysTick read
    // gives us a higher number than the first read, we know that it wrapped
    // somewhere between the two reads so our seconds and nanoseconds
    // snapshots are suspect. If this occurs, we go round again. Note that
    // it is not sufficient merely to read the values with interrupts disabled
    // since the SysTick counter keeps counting regardless of whether or not
    // the wrap interrupt has been serviced.
    //
    do
    {
        ulTime1 = SysTickValueGet();
        ulSeconds = g_ulSystemTimeSeconds;
        ulNanoseconds = g_ulSystemTimeNanoSeconds;
        ulPeriod = SysTickPeriodGet();
        ulTime2 = SysTickValueGet();

#ifdef DEBUG
        //
        // In debug builds, keep track of the number of times this function was
        // called just as the SysTick wrapped.
        //
        if(ulTime2 > ulTime1)
        {
            g_ulSysTickWrapDetect++;
            g_ulSysTickWrapTime = ulSeconds;
        }
#endif
    }
    while(ulTime2 > ulTime1);

    //
    // Fill in the seconds field from the snapshot we just took.
    //
    time->seconds = ulSeconds;

    //
    // Fill in the nanoseconds field from the snapshots.
    //
    time->nanoseconds = (ulNanoseconds + (ulPeriod - ulTime2) * TICKNS);

    //
    // Adjust for any case where we accumulate more than 1 second's worth of
    // nanoseconds.
    //
    if(time->nanoseconds >= 1000000000)
    {
#ifdef DEBUG
        g_ulGetTimeWrapCount++;
#endif
        time->seconds++;
        time->nanoseconds -= 1000000000;
    }
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
    sys_prot_t sProt;

    //
    // Update the System Tick Handler time values from the given PTPd time
    // (fine-tuning is handled in the System Tick handler). We need to update
    // these variables with interrupts disabled since the update must be
    // atomic.
    //
#ifdef DEBUG
    UARTprintf("Setting time %d.%09d\n", time->seconds, time->nanoseconds);
#endif
    sProt = sys_arch_protect();
    g_ulSystemTimeSeconds = time->seconds;
    g_ulSystemTimeNanoSeconds = time->nanoseconds;
    sys_arch_unprotect(sProt);
}

//*****************************************************************************
//
// Get the RX Timestamp. This is called from the lwIP low_level_input function
// when configured to include PTPd support.
//
//*****************************************************************************
void
getRxTime(TimeInternal *psRxTime)
{
    //
    // Get the current IEEE1588 time.
    //
    getTime(psRxTime);

    return;
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
    g_ulSystemTickHigh = ulTemp % TICKNS;

    //
    // Set the reload value.
    //
    g_ulNewSystemTickReload = ulTemp / TICKNS;

    //
    // Return.
    //
    return(TRUE);
}
