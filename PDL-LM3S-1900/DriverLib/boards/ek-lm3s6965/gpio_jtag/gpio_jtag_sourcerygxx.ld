/******************************************************************************
 *
 * gpio_jtag.ld - Scatter file for Sourcery G++
 *
 * Copyright (c) 2006-2007 Luminary Micro, Inc.  All rights reserved.
 * 
 * Software License Agreement
 * 
 * Luminary Micro, Inc. (LMI) is supplying this software for use solely and
 * exclusively on LMI's microcontroller products.
 * 
 * The software is owned by LMI and/or its suppliers, and is protected under
 * applicable copyright laws.  All rights are reserved.  You may not combine
 * this software with "viral" open-source software in order to form a larger
 * program.  Any use in violation of the foregoing restrictions may subject
 * the user to criminal sanctions under applicable laws, as well as to civil
 * liability for the breach of the terms and conditions of this license.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 * 
 * This is part of revision 1900 of the Stellaris Peripheral Driver Library.
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Define the end of the heap space, which determines the beginning of the
 * stack space.
 *
 *****************************************************************************/
__cs3_heap_end = __cs3_region_end_ram - 256;

/******************************************************************************
 *
 * Define the interrupt handlers used by the application.
 *
 *****************************************************************************/
__cs3_isr_gpio_f = GPIOFIntHandler;
