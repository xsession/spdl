//*****************************************************************************
//
// enet.c - lwIP Intialization and Application run-time code for static IP.
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
#include "../../../src/ethernet.h"
#include "../../../src/interrupt.h"
#include "../../../src/gpio.h"
#include "../../../src/flash.h"
#include "../../../src/sysctl.h"
#include "../../../utils/ustdlib.h"
#include "enet.h"
#include "globals.h"
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
// Timers for lwIP TCP/IP stack.
//
//*****************************************************************************
static unsigned long g_ulTCPFastTimer = 0;
static unsigned long g_ulTCPSlowTimer = 0;
static unsigned long g_ulARPTimer = 0;
#if LWIP_DHCP
static unsigned long g_ulDHCPCoarseTimer = 0;
static unsigned long g_ulDHCPFineTimer = 0;
static unsigned long g_ulDHCPTimeoutTimer = 0;
#endif

//*****************************************************************************
//
// Network Interface Structure.
//
//*****************************************************************************
static struct netif g_EMAC_if;

//*****************************************************************************
//
// Flag to indicate if Ethernet controller has been initialized.
//
//*****************************************************************************
static volatile tBoolean g_bEnetInit = false;

//*****************************************************************************
//
// External References
//
//*****************************************************************************
extern err_t ethernetif_init( struct netif *netif );
extern void ethernetif_input(struct netif *netif);
extern void httpd_init(void);

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
        // Flag the interrupt condition and disable the interrupt.
        //
        HWREGBITW(&g_ulFlags, FLAG_ENET_RXPKT) = 1;
        EthernetIntDisable(ETH_BASE, ETH_INT_RX);
    }

    //
    // Check if TX Interrupt Occurred.
    //
    if(ulTemp & ETH_INT_TX)
    {
        //
        // Flag the interrupt condition and disable the interrupt.
        //
        HWREGBITW(&g_ulFlags, FLAG_ENET_TXPKT) = 1;
        EthernetIntDisable(ETH_BASE, ETH_INT_TX);
    }
}

//*****************************************************************************
//
// Returns the current IP address.
//
//*****************************************************************************
unsigned long
EnetGetIPAddr(void)
{
    //
    // Return the current IP address.
    //
    return(g_EMAC_if.ip_addr.addr);
}

//*****************************************************************************
//
// Initializes the lwIP TCP/IP stack and Stellaris Ethernet controller for
// operation.
//
//*****************************************************************************
void
EnetInit(void)
{
    struct ip_addr xIpAddr, xNetMask, xGateway;
    unsigned long ulUser0, ulUser1;
    unsigned char pucMACArray[8];

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
    // Configure the hardware MAC address for Ethernet Controller
    // filtering of incoming packets.
    //
    // For the LM3S6965 Evaluation Kit, the MAC address will be stored in the
    // non-volatile USER0 and USER1 registers.  These registers can be read
    // using the FlashUserGet function, as illustrated below.
    //
    FlashUserGet(&ulUser0, &ulUser1);

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
#if LWIP_DHCP
    IP4_ADDR(&xIpAddr,0,0,0,0);
    IP4_ADDR(&xNetMask,0,0,0,0);
    IP4_ADDR(&xGateway,0,0,0,0);
#else
    IP4_ADDR(&xIpAddr, DEFAULT_IPADDR0, DEFAULT_IPADDR1, DEFAULT_IPADDR2,
             DEFAULT_IPADDR3);
    IP4_ADDR(&xNetMask, DEFAULT_NET_MASK0, DEFAULT_NET_MASK1,
             DEFAULT_NET_MASK2, DEFAULT_NET_MASK3);
    IP4_ADDR(&xGateway, DEFAULT_GATEWAY_ADDR0, DEFAULT_GATEWAY_ADDR1,
             DEFAULT_GATEWAY_ADDR2, DEFAULT_GATEWAY_ADDR3);
#endif
    netif_add(&g_EMAC_if, &xIpAddr, &xNetMask, &xGateway, NULL,
              ethernetif_init, ip_input);
    netif_set_default(&g_EMAC_if);
#if LWIP_DHCP
    dhcp_start(&g_EMAC_if);
#endif

    //
    // Bring the interface up
    //
    netif_set_up(&g_EMAC_if);

    //
    // Initialize the sample web server.
    //
    httpd_init();

    //
    // Initialization has been completed.
    //
    g_bEnetInit = true;
}

//*****************************************************************************
//
// Application runtime code that should be called periodically to run the
// varios TCP/IP stack timers.
//
//*****************************************************************************
void
EnetTick(unsigned long ulTickMS)
{
    //
    // Check to see if we have been initialized.
    //
    if(g_bEnetInit == false)
    {
        return;
    }

    //
    // Increment the TCP/IP timers.
    //
    g_ulTCPFastTimer += ulTickMS;
    g_ulTCPSlowTimer += ulTickMS;
    g_ulARPTimer += ulTickMS;
#if LWIP_DHCP
    g_ulDHCPCoarseTimer += ulTickMS;
    g_ulDHCPFineTimer += ulTickMS;
#endif

    //
    // Check and process incoming packets.
    //
    if(HWREGBITW(&g_ulFlags, FLAG_ENET_RXPKT) == 1)
    {
        HWREGBITW(&g_ulFlags, FLAG_ENET_RXPKT) = 0;
        ethernetif_input(&g_EMAC_if);
        EthernetIntEnable(ETH_BASE, ETH_INT_RX);
    }

    //
    // Check and process outgoing packets.
    //
    if(HWREGBITW(&g_ulFlags, FLAG_ENET_TXPKT) == 1)
    {
        HWREGBITW(&g_ulFlags, FLAG_ENET_TXPKT) = 0;
        EthernetIntEnable(ETH_BASE, ETH_INT_TX);
    }

    //
    // Check and process the ARP timer.
    //
    if(g_ulARPTimer >= ARP_TMR_INTERVAL)
    {
        g_ulARPTimer = 0;
        etharp_tmr();
    }

    //
    // Check and process the TCP slow timer.
    //
    if(g_ulTCPSlowTimer >= TCP_SLOW_INTERVAL)
    {
        g_ulTCPSlowTimer = 0;
        tcp_slowtmr();
    }

    //
    // Check and process the TCP fast timer.
    //
    if(g_ulTCPFastTimer >= TCP_FAST_INTERVAL)
    {
        g_ulTCPFastTimer = 0;
        tcp_fasttmr();
    }

    //
    // If DHCP is enabled/active, run the timers.
    //
#if LWIP_DHCP
    if(NULL != g_EMAC_if.dhcp)
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
        if( !(g_EMAC_if.dhcp->state == DHCP_BOUND) &&
             (g_ulDHCPTimeoutTimer > (DHCP_EXPIRE_TIMER_SECS * 1000)) )
        {
            struct ip_addr xIpAddr, xNetMask, xGateway;

            //
            // Disable the DHPC process.
            //
            dhcp_stop(&g_EMAC_if);

            //
            // Program the default IP settings.
            //
            IP4_ADDR(&xIpAddr, DEFAULT_IPADDR0, DEFAULT_IPADDR1,
                     DEFAULT_IPADDR2, DEFAULT_IPADDR3);
            IP4_ADDR(&xNetMask, DEFAULT_NET_MASK0, DEFAULT_NET_MASK1,
                     DEFAULT_NET_MASK2, DEFAULT_NET_MASK3);
            IP4_ADDR(&xGateway, DEFAULT_GATEWAY_ADDR0, DEFAULT_GATEWAY_ADDR1,
                    DEFAULT_GATEWAY_ADDR2, DEFAULT_GATEWAY_ADDR3);
            netif_set_ipaddr(&g_EMAC_if, &xIpAddr);
            netif_set_gw(&g_EMAC_if, &xGateway);
            netif_set_netmask(&g_EMAC_if, &xNetMask);
        }

        //
        // Update the DHCP timeout timer.
        //
        else
        {
            //
            // Increment the DHCP timeout timer.
            //
            g_ulDHCPTimeoutTimer += ulTickMS;
        }
    }
#endif
}
