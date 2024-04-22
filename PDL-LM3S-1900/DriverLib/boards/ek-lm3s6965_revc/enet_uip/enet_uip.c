//*****************************************************************************
//
// enet_uip.c - Sample WebServer Application for Ethernet Demo
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

#include "../../../hw_ints.h"
#include "../../../hw_memmap.h"
#include "../../../hw_types.h"
#include "../../../src/debug.h"
#include "../../../src/interrupt.h"
#include "../../../src/sysctl.h"
#include "../../../src/systick.h"
#include "../../../src/ethernet.h"
#include "../../../src/flash.h"
#include "../../../src/gpio.h"
#include "../../../src/ethernet.h"
#include "../../../utils/diag.h"
#include "../rit128x96x4.h"
#include "uip.h"
#include "uip_arp.h"
#include "httpd.h"

//*****************************************************************************
//
//! \addtogroup ek_lm3s6965_revc_list
//! <h1>Ethernet with uIP (enet_uip)</h1>
//!
//! This example application demonstrates the operation of the Stellaris
//! Ethernet controller using the uIP TCP/IP Stack.  A basic web site is served
//! over the Ethernet port, located at link local address 169.254.19.63.  If a
//! node on the network has already chosen this link local address, nothing is
//! done by the application to choose another address and a conflict will
//! occur.  The web site displays a few lines of text, and a counter that
//! increments each time the page is sent.
//!
//! For additional details on uIP, refer to the uIP web page at:
//! http://www.sics.se/~adam/uip/
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
// Macro for accessing the Ethernet header information in the buffer.
//
//*****************************************************************************
#define BUF                     ((struct uip_eth_hdr *)&uip_buf[0])

//*****************************************************************************
//
// A set of flags.  The flag bits are defined as follows:
//
//     0 -> An indicator that a SysTick interrupt has occurred.
//     1 -> An RX Packet has been received.
//
//*****************************************************************************
#define FLAG_SYSTICK            0
#define FLAG_RXPKT              1
static volatile unsigned long g_ulFlags;

//*****************************************************************************
//
// Default TCP/IP Settings for this application.
//
// Default to Link Local address ... (169.254.1.0 to 169.254.254.255).  Note:
// This application does not implement the Zeroconf protocol.  No ARP query is
// issued to determine if this static IP address is already in use.
//
//*****************************************************************************
#ifndef DEFAULT_IPADDR0
#define DEFAULT_IPADDR0         169
#endif

#ifndef DEFAULT_IPADDR1
#define DEFAULT_IPADDR1         254
#endif

#ifndef DEFAULT_IPADDR2
#define DEFAULT_IPADDR2         19
#endif

#ifndef DEFAULT_IPADDR3
#define DEFAULT_IPADDR3         63
#endif

#ifndef DEFAULT_NETMASK0
#define DEFAULT_NETMASK0        255
#endif

#ifndef DEFAULT_NETMASK1
#define DEFAULT_NETMASK1        255
#endif

#ifndef DEFAULT_NETMASK2
#define DEFAULT_NETMASK2        0
#endif

#ifndef DEFAULT_NETMASK3
#define DEFAULT_NETMASK3        0
#endif

//*****************************************************************************
//
// UIP Timers (in MS)
//
//*****************************************************************************
#define UIP_PERIODIC_TIMER_MS   500
#define UIP_ARP_TIMER_MS        10000

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
// The interrupt handler for the Ethernet interrupt.
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
    // Check to see if an RX Interrupt has occured.
    //
    if(ulTemp & ETH_INT_RX)
    {
        //
        // Indicate that a packet has been received.
        //
        HWREGBITW(&g_ulFlags, FLAG_RXPKT) = 1;

        //
        // Disable Ethernet RX Interrupt.
        //
        EthernetIntDisable(ETH_BASE, ETH_INT_RX);
    }
}

//*****************************************************************************
//
// Display a uIP type IP Address.
//
//*****************************************************************************
static void
DisplayIPAddress(uip_ipaddr_t ipaddr, unsigned long ulCol, unsigned long ulRow)
{
    char pucBuf[16];
    int iIndex = 0;
    unsigned char *pucTemp = (unsigned char *)ipaddr;
    unsigned char ucValue;

    //
    // Process first byte of IP Address.
    //
    ucValue = pucTemp[0];
    if(ucValue > 9)
    {
        if(ucValue > 99)
        {
            pucBuf[iIndex++] = '0' + (ucValue / 100);
            ucValue %= 100;
        }
        pucBuf[iIndex++] = '0' + (ucValue / 10);
        ucValue %= 10;
    }
    pucBuf[iIndex++] = '0' + (ucValue);
    pucBuf[iIndex++] = '.';

    //
    // Process second byte of IP Address.
    //
    ucValue = pucTemp[1];
    if(ucValue > 9)
    {
        if(ucValue > 99)
        {
            pucBuf[iIndex++] = '0' + (ucValue / 100);
            ucValue %= 100;
        }
        pucBuf[iIndex++] = '0' + (ucValue / 10);
        ucValue %= 10;
    }
    pucBuf[iIndex++] = '0' + (ucValue);
    pucBuf[iIndex++] = '.';

    //
    // Process third byte of IP Address.
    //
    ucValue = pucTemp[2];
    if(ucValue > 9)
    {
        if(ucValue > 99)
        {
            pucBuf[iIndex++] = '0' + (ucValue / 100);
            ucValue %= 100;
        }
        pucBuf[iIndex++] = '0' + (ucValue / 10);
        ucValue %= 10;
    }
    pucBuf[iIndex++] = '0' + (ucValue);
    pucBuf[iIndex++] = '.';

    //
    // Process last byte of IP Address.
    //
    ucValue = pucTemp[3];
    if(ucValue > 9)
    {
        if(ucValue > 99)
        {
            pucBuf[iIndex++] = '0' + (ucValue / 100);
            ucValue %= 100;
        }
        pucBuf[iIndex++] = '0' + (ucValue / 10);
        ucValue %= 10;
    }
    pucBuf[iIndex++] = '0' + (ucValue);
    pucBuf[iIndex] = 0;

    //
    // Display the string.
    //
    RIT128x96x4StringDraw(pucBuf, ulCol, ulRow, 15);
}

//*****************************************************************************
//
// This example demonstrates the use of the Ethernet Controller with the uIP
// TCP/IP stack.
//
//*****************************************************************************
int
main(void)
{
    uip_ipaddr_t ipaddr;
    static struct uip_eth_addr sTempAddr;
    long lPeriodicTimer, lARPTimer, lPacketLength;
    unsigned long ulUser0, ulUser1;
    unsigned long ulTemp;

    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);

    //
    // Initialize the OLED display.
    //
    RIT128x96x4Init(1000000);
    RIT128x96x4StringDraw("Ethernet with uIP", 12, 0, 15);

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
    // Intialize the Ethernet Controller and disable all Ethernet Controller
    // interrupt sources.
    //
    EthernetIntDisable(ETH_BASE, (ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER |
                       ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER | ETH_INT_RX));
    ulTemp = EthernetIntStatus(ETH_BASE, false);
    EthernetIntClear(ETH_BASE, ulTemp);

    //
    // Initialize the Ethernet Controller for operation.
    //
    EthernetInitExpClk(ETH_BASE, SysCtlClockGet());

    //
    // Configure the Ethernet Controller for normal operation.
    // - Full Duplex
    // - TX CRC Auto Generation
    // - TX Padding Enabled
    //
    EthernetConfigSet(ETH_BASE, (ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN |
                                 ETH_CFG_TX_PADEN));

    //
    // Enable the Ethernet Controller.
    //
    EthernetEnable(ETH_BASE);

    //
    // Enable the Ethernet interrupt.
    //
    IntEnable(INT_ETH);

    //
    // Enable the Ethernet RX Packet interrupt source.
    //
    EthernetIntEnable(ETH_BASE, ETH_INT_RX);

    //
    // Enable all processor interrupts.
    //
    IntMasterEnable();

    //
    // Initialize the uIP TCP/IP stack.
    //
    uip_init();
    uip_ipaddr(ipaddr, DEFAULT_IPADDR0, DEFAULT_IPADDR1, DEFAULT_IPADDR2,
               DEFAULT_IPADDR3);
    uip_sethostaddr(ipaddr);
    DisplayIPAddress(ipaddr, 18, 8);
    uip_ipaddr(ipaddr, DEFAULT_NETMASK0, DEFAULT_NETMASK1, DEFAULT_NETMASK2,
               DEFAULT_NETMASK3);
    uip_setnetmask(ipaddr);

    //
    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.
    //
    // For the Ethernet Eval Kits, the MAC address will be stored in the
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
        RIT128x96x4StringDraw("MAC Address", 0, 16, 15);
        RIT128x96x4StringDraw("Not Programmed!", 0, 24, 15);
        DiagExit(2);
    }

    //
    // Convert the 24/24 split MAC address from NV ram into a 32/16 split MAC
    // address needed to program the hardware registers, then program the MAC
    // address into the Ethernet Controller registers.
    //
    sTempAddr.addr[0] = ((ulUser0 >>  0) & 0xff);
    sTempAddr.addr[1] = ((ulUser0 >>  8) & 0xff);
    sTempAddr.addr[2] = ((ulUser0 >> 16) & 0xff);
    sTempAddr.addr[3] = ((ulUser1 >>  0) & 0xff);
    sTempAddr.addr[4] = ((ulUser1 >>  8) & 0xff);
    sTempAddr.addr[5] = ((ulUser1 >> 16) & 0xff);

    //
    // Program the hardware with it's MAC address (for filtering).
    //
    EthernetMACAddrSet(ETH_BASE, (unsigned char *)&sTempAddr);
    uip_setethaddr(sTempAddr);

    //
    // Initialize the TCP/IP Application (e.g. web server).
    //
    httpd_init();

    //
    // Main Application Loop.
    //
    lPeriodicTimer = 0;
    lARPTimer = 0;
    while(true)
    {
        //
        // Wait for an event to occur.  This can be either a System Tick event,
        // or an RX Packet event.
        //
        while(!g_ulFlags)
        {
        }

        //
        // If SysTick, Clear the SysTick interrupt flag and increment the
        // timers.
        //
        if(HWREGBITW(&g_ulFlags, FLAG_SYSTICK) == 1)
        {
            HWREGBITW(&g_ulFlags, FLAG_SYSTICK) = 0;
            lPeriodicTimer += SYSTICKMS;
            lARPTimer += SYSTICKMS;
        }

        //
        // Check for an RX Packet and read it.
        //
        lPacketLength = EthernetPacketGetNonBlocking(ETH_BASE, uip_buf,
                                                     sizeof(uip_buf));
        if(lPacketLength > 0)
        {
            //
            // Set uip_len for uIP stack usage.
            //
            uip_len = (unsigned short)lPacketLength;

            //
            // Clear the RX Packet event and renable RX Packet interrupts.
            //
            if(HWREGBITW(&g_ulFlags, FLAG_RXPKT) == 1)
            {
                HWREGBITW(&g_ulFlags, FLAG_RXPKT) = 0;
                EthernetIntEnable(ETH_BASE, ETH_INT_RX);
            }

            //
            // Process incoming IP packets here.
            //
            if(BUF->type == htons(UIP_ETHTYPE_IP))
            {
                uip_arp_ipin();
                uip_input();

                //
                // If the above function invocation resulted in data that
                // should be sent out on the network, the global variable
                // uip_len is set to a value > 0.
                //
                if(uip_len > 0)
                {
                    uip_arp_out();
                    EthernetPacketPut(ETH_BASE, uip_buf, uip_len);
                    uip_len = 0;
                }
            }

            //
            // Process incoming ARP packets here.
            //
            else if(BUF->type == htons(UIP_ETHTYPE_ARP))
            {
                uip_arp_arpin();

                //
                // If the above function invocation resulted in data that
                // should be sent out on the network, the global variable
                // uip_len is set to a value > 0.
                //
                if(uip_len > 0)
                {
                    EthernetPacketPut(ETH_BASE, uip_buf, uip_len);
                    uip_len = 0;
                }
            }
        }

        //
        // Process TCP/IP Periodic Timer here.
        //
        if(lPeriodicTimer > UIP_PERIODIC_TIMER_MS)
        {
            lPeriodicTimer = 0;
            for(ulTemp = 0; ulTemp < UIP_CONNS; ulTemp++)
            {
                uip_periodic(ulTemp);

                //
                // If the above function invocation resulted in data that
                // should be sent out on the network, the global variable
                // uip_len is set to a value > 0.
                //
                if(uip_len > 0)
                {
                    uip_arp_out();
                    EthernetPacketPut(ETH_BASE, uip_buf, uip_len);
                    uip_len = 0;
                }
            }

#if UIP_UDP
            for(ulTemp = 0; ulTemp < UIP_UDP_CONNS; ulTemp++)
            {
                uip_udp_periodic(i);

                //
                // If the above function invocation resulted in data that
                // should be sent out on the network, the global variable
                // uip_len is set to a value > 0.
                //
                if(uip_len > 0)
                {
                    uip_arp_out();
                    EthernetPacketPut(ETH_BASE, uip_buf, uip_len);
                    uip_len = 0;
                }
            }
#endif // UIP_UDP
        }

        //
        // Process ARP Timer here.
        //
        if(lARPTimer > UIP_ARP_TIMER_MS)
        {
            lARPTimer = 0;
            uip_arp_timer();
        }
    }
}
