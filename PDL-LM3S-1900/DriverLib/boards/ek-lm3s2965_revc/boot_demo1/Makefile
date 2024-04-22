#******************************************************************************
#
# Makefile - Rules for building the boot_demo1 example.
#
# Copyright (c) 2007 Luminary Micro, Inc.  All rights reserved.
# 
# Software License Agreement
# 
# Luminary Micro, Inc. (LMI) is supplying this software for use solely and
# exclusively on LMI's microcontroller products.
# 
# The software is owned by LMI and/or its suppliers, and is protected under
# applicable copyright laws.  All rights are reserved.  You may not combine
# this software with "viral" open-source software in order to form a larger
# program.  Any use in violation of the foregoing restrictions may subject
# the user to criminal sanctions under applicable laws, as well as to civil
# liability for the breach of the terms and conditions of this license.
# 
# THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
# OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
# LMI SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
# CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
# 
# This is part of revision 1900 of the Stellaris Peripheral Driver Library.
#
#******************************************************************************

#
# Defines the part type that this project uses.
#
PART=LM3S2965

#
# The base directory for DriverLib.
#
ROOT=../../..

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=..
VPATH+=../../../boot_loader

#
# Where to find header files that do not live in the source directory.
#
IPATH=.

#
# The default rule, which causes the boot_demo1 example to be built.
#
all: ${COMPILER}
all: ${COMPILER}/boot_demo1.axf
all: ${COMPILER}/bl_boot_demo1.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} settings
	@rm -rf ${wildcard *~} ${wildcard *.Bak} ${wildcard *.dep}
	@rm -rf bl_config.inc

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir ${COMPILER}

#
# Rules for building the boot_demo1 example.
#
${COMPILER}/boot_demo1.axf: ${COMPILER}/boot_demo1.o
${COMPILER}/boot_demo1.axf: ${COMPILER}/rit128x96x4.o
${COMPILER}/boot_demo1.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/boot_demo1.axf: ../../../src/${COMPILER}/libdriver.a
ifeq (${COMPILER}, gcc)
${COMPILER}/boot_demo1.axf: boot_demo1.ld
endif
ifeq (${COMPILER}, ewarm)
${COMPILER}/boot_demo1.axf: boot_demo1.xcl
endif
ROBASE_boot_demo1=0x800
SCATTERgcc_boot_demo1=boot_demo1.ld
SCATTERewarm_boot_demo1=boot_demo1.xcl
SCATTERsourcerygxx_boot_demo1=lm3s2965-rom.ld
ENTRY_boot_demo1=ResetISR
LDFLAGSsourcerygxx_boot_demo1=-T boot_demo1_sourcerygxx.ld

#
# Rules for building the Stellaris Boot Loader.
#
${COMPILER}/bl_boot_demo1.axf: ${COMPILER}/bl_autobaud.o
${COMPILER}/bl_boot_demo1.axf: ${COMPILER}/bl_i2c.o
${COMPILER}/bl_boot_demo1.axf: ${COMPILER}/bl_main.o
${COMPILER}/bl_boot_demo1.axf: ${COMPILER}/bl_packet.o
${COMPILER}/bl_boot_demo1.axf: ${COMPILER}/bl_ssi.o
${COMPILER}/bl_boot_demo1.axf: ${COMPILER}/bl_startup_${COMPILER}.o
${COMPILER}/bl_boot_demo1.axf: ${COMPILER}/bl_uart.o
SCATTERgcc_bl_boot_demo1=../../../boot_loader/bl_link.ld
SCATTERewarm_bl_boot_demo1=../../../boot_loader/bl_link.xcl
SCATTERrvmdk_bl_boot_demo1=../../../boot_loader/bl_link.sct
SCATTERsourcerygxx_bl_boot_demo1=../../../boot_loader/bl_link.ld
ENTRY_bl_boot_demo1=ResetISR
LDFLAGSsourcerygxx_bl_boot_demo1=--entry ResetISR

#
# Since RV-MDK can not pass assembly source through the C preprocessor,
# generate an assembly include file from the C header file that contains the
# boot loader configuration.
#
ifeq (${COMPILER},rvmdk)
rvmdk/bl_startup_rvmdk.o: bl_config.inc

bl_config.inc: bl_config.h
	@armcc --device DLM -I . -E -o bl_config.inc -c \
	       ../../../boot_loader/bl_config.c
endif

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
