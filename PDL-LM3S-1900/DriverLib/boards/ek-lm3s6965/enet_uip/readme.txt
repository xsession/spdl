Ethernet with uIP TCP/IP Stack

This example application demonstrates the operation of the Stellaris Ethernet
Controller using the uIP TCP/IP Stack.  A basic web site is served over the
Ethernet port, located at link local address 169.254.19.63.  If a node on the
network has already chosen this link local address, nothing is done by the
application to choose another address and a conflict will occur.  The web site
displays a few lines of text, and a counter that increments each time the page
is sent.

For additional details on uIP, refer to the uIP web page at:
http://www.sics.se/~adam/uip/

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
