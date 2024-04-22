//*****************************************************************************
//
// enet_lwip.c - Sample WebServer Application using lwIP.
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
#include "../rit128x96x4.h"
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "netif/etharp.h"

//*****************************************************************************
//
//! \addtogroup ek_lm3s8962_list
//! <h1>Ethernet with lwIP (enet_lwip)</h1>
//!
//! This example application demonstrates the operation of the Stellaris
//! Ethernet controller using the lwIP TCP/IP Stack.  DHCP is used to obtain an
//! Ethernet address.  If DHCP times out without obtaining an address, a static
//! IP address will be used.  The DHCP timeout and the default static IP are
//! easily configurable using macros.  The address that is selected will be
//! shown on the OLED display.
//!
//! The file system code will first check to see if an SD card has been plugged
//! into the microSD slot.  If so, all file requests from the web server will
//! be directed to the SD card.  Otherwise, a default set of pages served up
//! by an internal file system will be used.
//!
//! For additional details on lwIP, refer to the lwIP web page at:
//! http://www.sics.se/~adam/lwip/
//
//*****************************************************************************

//*****************************************************************************
//
// Defines for setting up the system clock.
//
//*****************************************************************************
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)
#define SYSTICKUS               (1000000 / SYSTICKHZ)
#define SYSTICKNS               (1000000000 / SYSTICKHZ)

//*****************************************************************************
//
// A set of flags.  The flag bits are defined as follows:
//
//     0 -> An indicator that a SysTick interrupt has occurred.
//     1 -> An RX Packet has been received.
//     2 -> An RX Packet has been received.
//
//*****************************************************************************
#define FLAG_SYSTICK            0
#define FLAG_RXPKT              1
#define FLAG_TXPKT              2
static volatile unsigned long g_ulFlags;

//*****************************************************************************
//
// External Application references.
//
//*****************************************************************************
extern void httpd_init(void);
extern err_t ethernetif_init(struct netif *netif);
extern void ethernetif_input(struct netif *netif);
extern void fs_init(void);
extern void fs_tick(unsigned long ulTickMS);

//*****************************************************************************
//
// Counters/Timers for lwIP.
//
//*****************************************************************************
unsigned long g_ulTCPFastTimer = 0;
unsigned long g_ulTCPSlowTimer = 0;
unsigned long g_ulARPTimer = 0;
unsigned long g_ulDHCPCoarseTimer = 0;
unsigned long g_ulDHCPFineTimer = 0;
static struct netif g_sEMAC_if;
unsigned long g_ulDHCPTimeoutTimer = 0;

//*****************************************************************************
//
// Default TCP/IP Address Configuration.  Static IP Configuration is used if
// DHCP times out.  Note:  This is in the Link Local address range
// (169.254.x.y).
//
//*****************************************************************************
//
// The Default IP address to be used.
//
#ifndef DEFAULT_IPADDR0
#define DEFAULT_IPADDR0 169
#endif

#ifndef DEFAULT_IPADDR1
#define DEFAULT_IPADDR1 254
#endif

#ifndef DEFAULT_IPADDR2
#define DEFAULT_IPADDR2 19
#endif

#ifndef DEFAULT_IPADDR3
#define DEFAULT_IPADDR3 63
#endif

//
// The Default Gateway address to be used.
//
#ifndef DEFAULT_GATEWAY_ADDR0
#define DEFAULT_GATEWAY_ADDR0 0
#endif

#ifndef DEFAULT_GATEWAY_ADDR1
#define DEFAULT_GATEWAY_ADDR1 0
#endif

#ifndef DEFAULT_GATEWAY_ADDR2
#define DEFAULT_GATEWAY_ADDR2 0
#endif

#ifndef DEFAULT_GATEWAY_ADDR3
#define DEFAULT_GATEWAY_ADDR3 0
#endif

//
// The Default Network mask to be used.
//
#ifndef DEFAULT_NET_MASK0
#define DEFAULT_NET_MASK0 255
#endif

#ifndef DEFAULT_NET_MASK1
#define DEFAULT_NET_MASK1 255
#endif

#ifndef DEFAULT_NET_MASK2
#define DEFAULT_NET_MASK2 0
#endif

#ifndef DEFAULT_NET_MASK3
#define DEFAULT_NET_MASK3 0
#endif

//*****************************************************************************
//
// Timeout for DHCP address request (in seconds).
//
//*****************************************************************************
#ifndef DHCP_EXPIRE_TIMER_SECS
#define DHCP_EXPIRE_TIMER_SECS      45
#endif

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
}

//*****************************************************************************
//
// Ethernet Interrupt handler.
//
//*****************************************************************************
void
EthernetIntHandler(void)
{
    unsigned long ulTemp;

    //
    // Read and Clear the interrupt.
    //
    ulTemp = EthernetIntStatus(ETH_BASE, false);
    EthernetIntClear(ETH_BASE, ulTemp);

    //
    // Check if RX Interrupt Occurred.
    //
    if(ulTemp & ETH_INT_RX)
    {
        //
        // Indicate that an RX Packet has been received.
        //
        HWREGBITW(&g_ulFlags, FLAG_RXPKT) = 1;

        //
        // Disable Ethernet RX Packet Interrupt.
        //
        EthernetIntDisable(ETH_BASE, ETH_INT_RX);
    }

    //
    // Check if TX Interrupt Occurred.
    //
    if(ulTemp & ETH_INT_TX)
    {
        //
        // Indicate that an RX Packet has been received.
        //
        HWREGBITW(&g_ulFlags, FLAG_TXPKT) = 1;

        //
        // Disable Ethernet RX Packet Interrupt.
        //
        EthernetIntDisable(ETH_BASE, ETH_INT_TX);
    }
}

//*****************************************************************************
//
// Display an lwIP type IP Address.
//
//*****************************************************************************
void
DisplayIPAddress(unsigned long ipaddr, unsigned long ulCol,
                 unsigned long ulRow)
{
    char pucBuf[16];
    int iIndex = 0;
    int iTemp;
    unsigned char *pucTemp = (unsigned char *)&ipaddr;

    //
    // Convert the "long" IP Address into a string, one byte at a time.
    //
    iTemp = usprintf(&pucBuf[iIndex], "%d.", pucTemp[0]);
    iIndex += iTemp;
    iTemp = usprintf(&pucBuf[iIndex], "%d.", pucTemp[1]);
    iIndex += iTemp;
    iTemp = usprintf(&pucBuf[iIndex], "%d.", pucTemp[2]);
    iIndex += iTemp;
    iTemp = usprintf(&pucBuf[iIndex], "%d", pucTemp[3]);
    iIndex += iTemp;
    pucBuf[iIndex] = 0;

    //
    // Display the string.
    //
    RIT128x96x4StringDraw(pucBuf, ulCol, ulRow, 15);
}

//*****************************************************************************
//
// Should be called by the top-level application to perform the needed lwIP
// TCP/IP initialization
//
//*****************************************************************************
void
lwip_init(void)
{
    struct ip_addr xIpAddr, xNetMast, xGateway;

    //
    // Low-Level initialization of the lwIP stack modules.
    //
    stats_init();
    sys_init();
    mem_init();
    memp_init();
    pbuf_init();
    etharp_init();
    ip_init();
    udp_init();
    tcp_init();
    netif_init();

    //
    // Create, Configure and Add the Ethernet Controller Interface.
    //
    IP4_ADDR(&xIpAddr,0,0,0,0);
    IP4_ADDR(&xNetMast,0,0,0,0);
    IP4_ADDR(&xGateway,0,0,0,0);
    netif_add(&g_sEMAC_if,
              &xIpAddr,
              &xNetMast,
              &xGateway,
              NULL,
              ethernetif_init,
              ip_input);
    netif_set_default(&g_sEMAC_if);
    dhcp_start(&g_sEMAC_if);
    RIT128x96x4Enable(1000000);
    RIT128x96x4StringDraw("Waiting for DHCP", 0, 16, 15);
    RIT128x96x4StringDraw("<                   > ", 0, 24, 15);
    RIT128x96x4Disable();

    //
    // Bring the interface up.
    //
    netif_set_up(&g_sEMAC_if);
}

//*****************************************************************************
//
// Should be called by the top-level application every system tick, with the
// number of MS per system tick, to run lwIP timers, etc.
//
//*****************************************************************************
void
lwip_tick(unsigned long ulTickMS)
{
    static tBoolean bDisplayIP = false;
    static tBoolean bIPDisplayed = false;

    //
    // Increment the assorted timers.
    //
    g_ulTCPFastTimer += ulTickMS;
    g_ulTCPSlowTimer += ulTickMS;
    g_ulARPTimer += ulTickMS;
    g_ulDHCPCoarseTimer += ulTickMS;
    g_ulDHCPFineTimer += ulTickMS;

    //
    // Check and process incoming packets
    //
    ethernetif_input(&g_sEMAC_if);

    //
    // Check ARP Timer.
    //
    if(g_ulARPTimer >= ARP_TMR_INTERVAL)
    {
        g_ulARPTimer = 0;
        etharp_tmr();
    }

    //
    // Check TCP Slow Timer.
    //
    if(g_ulTCPSlowTimer >= TCP_SLOW_INTERVAL)
    {
        g_ulTCPSlowTimer = 0;
        tcp_slowtmr();
    }

    //
    // Check TCP Fast Timer.
    //
    if(g_ulTCPFastTimer >= TCP_FAST_INTERVAL)
    {
        g_ulTCPFastTimer = 0;
        tcp_fasttmr();
    }

    //
    // If DHCP is enabled/active, run the timers.
    //
    if(NULL != g_sEMAC_if.dhcp)
    {
        //
        // Check DCHP Coarse Timer.
        //
        if(g_ulDHCPCoarseTimer >= (DHCP_COARSE_TIMER_SECS * 1000))
        {
            g_ulDHCPCoarseTimer = 0;
            dhcp_coarse_tmr();
        }

        //
        // Check DCHP Fine Timer.
        //
        if(g_ulDHCPFineTimer >= DHCP_FINE_TIMER_MSECS)
        {
            g_ulDHCPFineTimer = 0;
            dhcp_fine_tmr();
        }

        //
        // Check to see if DHCP is bound.
        //
        if(g_sEMAC_if.dhcp->state == DHCP_BOUND)
        {
            bDisplayIP = true;
        }

        //
        // Check to see if if the DHCP process has taken too long.
        //
        else if(g_ulDHCPTimeoutTimer > (DHCP_EXPIRE_TIMER_SECS * 1000))
        {
            struct ip_addr xIpAddr, xNetMask, xGateway;

            //
            // Disable the DHPC process.
            //
            dhcp_stop(&g_sEMAC_if);

            //
            // Program the default IP settings.
            //
            IP4_ADDR(&xIpAddr, DEFAULT_IPADDR0, DEFAULT_IPADDR1,
                     DEFAULT_IPADDR2, DEFAULT_IPADDR3);
            IP4_ADDR(&xNetMask, DEFAULT_NET_MASK0, DEFAULT_NET_MASK1,
                     DEFAULT_NET_MASK2, DEFAULT_NET_MASK3);
            IP4_ADDR(&xGateway, DEFAULT_GATEWAY_ADDR0, DEFAULT_GATEWAY_ADDR1,
                    DEFAULT_GATEWAY_ADDR2, DEFAULT_GATEWAY_ADDR3);
            netif_set_ipaddr(&g_sEMAC_if, &xIpAddr);
            netif_set_gw(&g_sEMAC_if, &xGateway);
            netif_set_netmask(&g_sEMAC_if, &xNetMask);

            //
            // Prompt a display of the IP settings.
            //
            bDisplayIP = true;
        }

        //
        // Display DHCP Status.
        //
        else
        {
            static int iColumn = 6;

            //
            // Increment the DHCP timeout timer.
            //
            g_ulDHCPTimeoutTimer += ulTickMS;

            //
            // Update status bar on the display.
            //
            RIT128x96x4Enable(1000000);
            if(iColumn < 12)
            {
                RIT128x96x4StringDraw("< ", 0, 24, 15);
                RIT128x96x4StringDraw("*",iColumn, 24, 7);
            }
            else
            {
                RIT128x96x4StringDraw(" *",iColumn - 6, 24, 7);
            }

            iColumn++;
            if(iColumn > 114)
            {
                iColumn = 6;
                RIT128x96x4StringDraw(" >", 114, 24, 15);
            }
            RIT128x96x4Disable();
        }
    }

    //
    // Check if DHCP has been bound.
    //
    if(bDisplayIP && !bIPDisplayed)
    {
        RIT128x96x4Enable(1000000);
        RIT128x96x4StringDraw("                       ", 0, 16, 15);
        RIT128x96x4StringDraw("                       ", 0, 24, 15);
        RIT128x96x4StringDraw("IP:   ", 0, 16, 15);
        RIT128x96x4StringDraw("MASK: ", 0, 24, 15);
        RIT128x96x4StringDraw("GW:   ", 0, 32, 15);
        DisplayIPAddress(g_sEMAC_if.ip_addr.addr, 36, 16);
        DisplayIPAddress(g_sEMAC_if.netmask.addr, 36, 24);
        DisplayIPAddress(g_sEMAC_if.gw.addr, 36, 32);
        RIT128x96x4Disable();
        bIPDisplayed = true;
    }
}

//*****************************************************************************
//
// This example demonstrates the use of the Ethernet Controller.
//
//*****************************************************************************
int
main(void)
{
    unsigned long ulUser0, ulUser1;
    unsigned char pucMACArray[8];

    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);

    //
    // Initialize the OLED display.
    //
    RIT128x96x4Init(1000000);
    RIT128x96x4StringDraw("Ethernet with lwIP", 12, 0, 15);

    //
    // Check for presence of Ethernet Controller.
    //
    if(!SysCtlPeripheralPresent(SYSCTL_PERIPH_ETH))
    {
        RIT128x96x4StringDraw("Ethernet Controller", 0, 16, 15);
        RIT128x96x4StringDraw("Not Found!", 0, 24, 15);
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
    // Configure SysTick for a periodic interrupt.
    //
    SysTickPeriodSet(SysCtlClockGet() / SYSTICKHZ);
    SysTickEnable();
    SysTickIntEnable();

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.
    //
    // For the LM3S6965 Evaluation Kit, the MAC address will be stored in the
    // non-volatile USER0 and USER1 registers.  These registers can be read
    // using the FlashUserGet function, as illustrated below.
    //
    FlashUserGet(&ulUser0, &ulUser1);
    if((ulUser0 == 0xffffffff) || (ulUser1 == 0xffffffff))
    {
        //
        // We should never get here.  This is an error if the MAC address
        // has not been programmed into the device.  Exit the program.
        //
        RIT128x96x4StringDraw("MAC Address", 0, 16, 15);
        RIT128x96x4StringDraw("Not Programmed!", 0, 24, 15);
        DiagExit(2);
    }

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split
    // MAC address needed to program the hardware registers, then program
    // the MAC address into the Ethernet Controller registers.
    //
    pucMACArray[0] = ((ulUser0 >>  0) & 0xff);
    pucMACArray[1] = ((ulUser0 >>  8) & 0xff);
    pucMACArray[2] = ((ulUser0 >> 16) & 0xff);
    pucMACArray[3] = ((ulUser1 >>  0) & 0xff);
    pucMACArray[4] = ((ulUser1 >>  8) & 0xff);
    pucMACArray[5] = ((ulUser1 >> 16) & 0xff);

    //
    // Program the hardware with it's MAC address (for filtering).
    //
    EthernetMACAddrSet(ETH_BASE, pucMACArray);

    //
    // Initialize the file system.
    //
    RIT128x96x4Disable();
    fs_init();

    //
    // Initialize all of the lwIP code, as needed, which will also initialize
    // the low-level Ethernet code.
    //
    lwip_init();

    //
    // Initialize a sample httpd server.
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
            // Run the file system tick.
            //
            fs_tick(SYSTICKMS);
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
    }
}
