//*****************************************************************************
//
// bl_startup_ewarm.S - Startup code for EWARM.
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

//*****************************************************************************
//
// Include the assember definitions used to make this code compiler
// independent.
//
//*****************************************************************************
#include "bl_config.h"

//*****************************************************************************
//
// This portion of the file goes into the vector section.
//
//*****************************************************************************
    rseg    INTVEC:DATA(2)

//*****************************************************************************
//
// The minimal vector table for a Cortex-M3 processor.
//
//*****************************************************************************
    export  __vector_table
__vector_table
    import  g_pulStack
    dcd     g_pulStack + (STACK_SIZE * 4)   // Offset 00: Initial stack pointer
    dcd     ResetISR - 0x20000000           // Offset 04: Reset handler
    dcd     NmiSR                           // Offset 08: NMI handler
    dcd     FaultISR                        // Offset 0C: Hard fault handler
    dcd     IntDefaultHandler               // Offset 10: MPU fault handler
    dcd     IntDefaultHandler               // Offset 14: Bus fault handler
    dcd     IntDefaultHandler               // Offset 18: Usage fault handler
    dcd     0                               // Offset 1C: Reserved
    dcd     0                               // Offset 20: Reserved
    dcd     0                               // Offset 24: Reserved
    dcd     0                               // Offset 28: Reserved
    dcd     UpdateHandler - 0x20000000      // Offset 2C: SVCall handler
#if defined(UART_ENABLE_UPDATE) && defined(UART_AUTOBAUD)
    dcd     IntDefaultHandler               // Offset 30: Debug monitor handler
    dcd     0                               // Offset 34: Reserved
    dcd     IntDefaultHandler               // Offset 38: PendSV handler
    dcd     IntDefaultHandler               // Offset 3C: SysTick handler
    import  GPIOIntHandler
    dcd     GPIOIntHandler                  // Offset 40: GPIO port A handler
#endif

//*****************************************************************************
//
// Define the DATA_Z segment so that it can be referred to later in order to
// find the beginning and end of the zero initialized memory.
//
//*****************************************************************************
    RSEG DATA_Z

//*****************************************************************************
//
// This portion of the file goes into the text section.
//
//*****************************************************************************
    rseg    CODE:CODE(2)

//*****************************************************************************
//
// Initialize the processor by copying the boot loader from flash to SRAM, zero
// filling the .bss section, and moving the vector table to the beginning of
// SRAM.  The return address is modified to point to the SRAM copy of the boot
// loader instead of the flash copy, resulting in a branch to the copy now in
// SRAM.
//
//*****************************************************************************
ProcessorInit
    //
    // Copy the code image from flash to SRAM.
    //
    movs    r0, #0x00000000
    ldr     r1, =0x20000000
    ldr     r2, =SFB(DATA_Z)
copy_loop
        ldr     r3, [r0], #4
        str     r3, [r1], #4
        cmp     r1, r2
        blt     copy_loop

    //
    // Zero fill the .bss section.
    //
    movs    r0, #0x00000000
    ldr     r2, =SFE(DATA_Z)
zero_loop
        str     r0, [r1], #4
        cmp     r1, r2
        blt     zero_loop

    //
    // Set the vector table pointer to the beginning of SRAM.
    //
    ldr     r0, =0xe000ed08
    ldr     r1, =0x20000000
    str     r1, [r0]

    //
    // Set the return address to the code just copied into SRAM.
    //
    orr     lr, lr, #0x20000000

    //
    // Return to the caller.
    //
    bx      lr

//*****************************************************************************
//
// The reset handler, which gets called when the processor starts.
//
//*****************************************************************************
    export  ResetISR
ResetISR
    //
    // Initialize the processor.
    //
    bl      ProcessorInit

    //
    // See if a an update should be performed.
    //
    import  CheckForceUpdate
    bl      CheckForceUpdate
    cbz     r0, CallApplication

    //
    // Configure the microcontroller.
    //
    import  ConfigureDevice
    bl      ConfigureDevice

    //
    // Branch to the update handler.
    //
    import  Updater
    b       Updater

    //
    // Call the application via the reset handler in its vector table.  Load
    // the address of the application vector table.
    //
CallApplication
    ldr     r0, =APP_START_ADDRESS

    //
    // Set the vector table address to the beginning of the application.
    //
    ldr     r1, =0xe000ed08
    str     r0, [r1]

    //
    // Load the stack pointer from the application's vector table.
    //
    ldr     r1, [r0]
    mov     sp, r1

    //
    // Load the initial PC from the application's vector table and branch to
    // the application's entry point.
    //
    ldr     r0, [r0, #4]
    bx      r0

//*****************************************************************************
//
// The update handler, which gets called when the application would like to
// start an update.
//
//*****************************************************************************
UpdateHandler
    //
    // Initialize the processor.
    //
    bl      ProcessorInit

    //
    // Load the stack pointer from the vector table.
    //
    movs    r0, #0x00000000
    ldr     r0, [r0]
    mov     sp, r0

    //
    // Branch to the update handler.
    //
    b       Updater

//*****************************************************************************
//
// The NMI handler.
//
//*****************************************************************************
NmiSR
    //
    // Loop forever since there is nothing that we can do about a NMI.
    //
    b       .

//*****************************************************************************
//
// The hard fault handler.
//
//*****************************************************************************
FaultISR
    //
    // Loop forever since there is nothing that we can do about a hard fault.
    //
    b       .

//*****************************************************************************
//
// The default interrupt handler.
//
//*****************************************************************************
IntDefaultHandler
    //
    // Loop forever since there is nothing that we can do about an unexpected
    // interrupt.
    //
    b       .

//*****************************************************************************
//
// This is the end of the file.
//
//*****************************************************************************
    end
