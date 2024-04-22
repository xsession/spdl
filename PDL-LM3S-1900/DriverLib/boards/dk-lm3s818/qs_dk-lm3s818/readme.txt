DK-LM3S818 Quickstart Application

This example uses the potentiometer on the development board to vary the rate
of a repetitive beep from the piezo buzzer, while the light sensor will vary
the frequency of the beep.  Turning the knob in one direction will result in
slower beeps while turning it in the other direction will result in faster
beeps.  The amount of light falling on the light sensor affects the frequency
of the beep.  The more light falling on the sensor the higher the pitch of the
beep.  The potentiometer setting along with the "note" representing the pitch
of the beep is displayed on the LCD, and a log of the readings is output on the
UART at 115,200, 8-n-1.  The push button can be used to turn the beeping noise
on and off; when off the LCD and UART still provide the settings.

In the default jumper configuration of the development board, the push button
will not actually mute the beep.  In order for this example to fully work, the
following jumper wire connections must be made: JP19 pin 2 to J6 pin 6.

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
