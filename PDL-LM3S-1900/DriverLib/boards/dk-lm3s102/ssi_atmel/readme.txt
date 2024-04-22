SSI

This example application uses the SSI master to communicate with the Atmel
AT25F1024A EEPROM that is on the development board.  The first 256 bytes of the
EEPROM are erased and then programmed with an incrementing sequence.  The data
is then read back to verify its correctness.  The transfer is managed by an
interrupt handler in response to the SSI interrupt; since a 256-byte read at a
1MHz SSI bus speed takes around 2ms, this allows a lot of other processing to
occur during the transfer (though that time is not utilized by this example).

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
