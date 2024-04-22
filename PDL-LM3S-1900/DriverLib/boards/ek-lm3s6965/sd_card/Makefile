#******************************************************************************
#
# Makefile - Rules for building the Sample SD Card Application.
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
PART=LM3S6965

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
VPATH+=../../../utils
VPATH+=../../../third_party/fatfs/src
VPATH+=../../../third_party/fatfs/port

#
# Where to find header files that do not live in the source directory.
#
IPATH=..
IPATH+=../../../third_party/fatfs/src

#
# The default rule, which causes the Sample SD Card Application to be built.
#
all: ${COMPILER}
all: ${COMPILER}/sd_card.axf

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} settings
	@rm -rf ${wildcard *~} ${wildcard *.Bak} ${wildcard *.dep}

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir ${COMPILER}

#
# Rules for building the Sample SD Card Application.
#
${COMPILER}/sd_card.axf: ${COMPILER}/${DEBUGGER}.o
${COMPILER}/sd_card.axf: ${COMPILER}/cmdline.o
${COMPILER}/sd_card.axf: ${COMPILER}/ff.o
${COMPILER}/sd_card.axf: ${COMPILER}/mmc-ek-lm3s6965.o
${COMPILER}/sd_card.axf: ${COMPILER}/sd_card.o
${COMPILER}/sd_card.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/sd_card.axf: ${COMPILER}/uartstdio.o
${COMPILER}/sd_card.axf: ../../../src/${COMPILER}/libdriver.a
SCATTERgcc_sd_card=../../../gcc/standalone.ld
SCATTERewarm_sd_card=../../../ewarm/standalone.xcl
SCATTERsourcerygxx_sd_card=lm3s6965-rom.ld
ENTRY_sd_card=ResetISR
CFLAGSewarm=-D STACK_SIZE=256
CFLAGSgcc=-D STACK_SIZE=256
CFLAGSrvmdk=-D STACK_SIZE=256
LDFLAGSsourcerygxx_sd_card=-T sd_card_sourcerygxx.ld

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
