GPIO

This example application uses LEDs connected to GPIO pins to create a "roving
eye" display.  Port B0-B3 are driven in a sequential manner to give the
illusion of an eye looking back and forth.

In order for this example to work properly, the ULED0 (JP22), ULED1 (JP23),
ULED2 (JP24), and ULED3 (JP25) jumpers must be installed on the board, and the
PB1 (JP1) jumper on the daughtercard must be set to pins 2 & 3.

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
