DK-LM3S102 Quickstart Application

This example uses the photocell on the development board to create a geiger
counter for visible light.  In bright light, the click rate (i.e. the count)
increases; in low light it decreases.  The light reading is also displayed on
the LCD, and a log of the readings is output on the UART at 115,200, 8-n-1.
The push button can be used to turn off the clicking noise on and off; when off
the LCD and UART still provide the light reading.

In the default jumper configuration of the development board, this example
actually samples the potentiometer and the push button will not work.  In order
for this example to fully work, the following jumper wire connections must be
made: JP3 pin 1 to JP5 pin 2 (requiring the removal of the jumper on JP5) and
JP19 pin 2 to J6 pin 6.

-------------------------------------------------------------------------------

Copyright (c) 2006-2007 Luminary Micro, Inc.  All rights reserved.

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
