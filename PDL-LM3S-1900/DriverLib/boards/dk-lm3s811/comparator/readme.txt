Comparator

This example application demonstrates the operation of the analog
comparator(s).  Comparator zero (which is present on all devices that have
analog comparators) is configured to compare its negative input to an
internally generated 1.65V reference and toggle the state of the LED on port B0
based on comparator change interrupts.  The LED will be turned on by the
interrupt handler when a rising edge on the comparator output is detected, and
will be turned off when a falling edge is detected.

In order for this example to work properly, the ULED0 (JP22) jumper must be
installed on the board.

-------------------------------------------------------------------------------

Copyright (c) 2005-2007 Luminary Micro, Inc.  All rights reserved.

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
