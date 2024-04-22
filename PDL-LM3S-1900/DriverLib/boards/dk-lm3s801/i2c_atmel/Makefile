#******************************************************************************
#
# Makefile - Rules for building the i2c_atmel example.
#
# Copyright (c) 2005-2007 Luminary Micro, Inc.  All rights reserved.
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
PART=LM3S801

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
VPATH=../../../utils
VPATH+=..

#
# The default rule, which causes the i2c_atmel example to be built.
#
all: ${COMPILER}
all: ${COMPILER}/i2c_atmel.axf

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
# Rules for building the i2c_atmel example.
#
${COMPILER}/i2c_atmel.axf: ${COMPILER}/${DEBUGGER}.o
${COMPILER}/i2c_atmel.axf: ${COMPILER}/i2c_atmel.o
${COMPILER}/i2c_atmel.axf: ${COMPILER}/pdc.o
${COMPILER}/i2c_atmel.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/i2c_atmel.axf: ../../../src/${COMPILER}/libdriver.a
SCATTERgcc_i2c_atmel=../../../gcc/standalone.ld
SCATTERewarm_i2c_atmel=../../../ewarm/standalone.xcl
SCATTERsourcerygxx_i2c_atmel=lm3s801-rom.ld
ENTRY_i2c_atmel=ResetISR
LDFLAGSsourcerygxx_i2c_atmel=-T i2c_atmel_sourcerygxx.ld

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
