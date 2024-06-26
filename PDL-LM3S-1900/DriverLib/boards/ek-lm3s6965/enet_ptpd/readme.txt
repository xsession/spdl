Ethernet IEEE 1588 (PTPd) with lwIP (enet_ptpd)

This example application demonstrates the operation of the Stellaris
Ethernet controller using the lwIP TCP/IP Stack.  DHCP is used to obtain an
Ethernet address.  If DHCP times out without obtaining an address, a static
IP address will be used.  The DHCP timeout and the default static IP are
easily configurable using macros.  The address that is selected will be
shown on the OLED display.

A default set of pages will be served up by an internal file system and
the httpd server.

The IEEE 1588 (PTP) software has been enabled in this code to synchronize
the internal clock to a network master clock source.

For additional details on lwIP, refer to the lwIP web page at:
http://www.sics.se/~adam/lwip/

For additional details on the PTPd software, refer to the PTPd web page at:
http://ptpd.sourceforge.net

-------------------------------------------------------------------------------

Copyright (c) 2007 Luminary Micro, Inc.  All rights reserved.

Software License Agreement

Luminary Micro, Inc. (LMI) is supplying this software for use solely and
exclusively on LMI's microcontroller products.

The software is owned by LMI and/or its suppliers, and is protected under
applicable copyright laws.  All rights are reserved.  You may not combine
this software with "viral" open-source software in order to form a larger
program.  Any use in violation of the foregoing restrictions may subject
the user to criminal sanctions under applicable laws, as well as to civil
liability for the breach of the terms and conditions of this license.

THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

This is part of revision 1900 of the Stellaris Peripheral Driver Library.
