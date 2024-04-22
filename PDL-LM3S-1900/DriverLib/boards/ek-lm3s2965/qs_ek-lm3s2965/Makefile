#******************************************************************************
#
# Makefile - Rules for building the qs_ek-lm3s2965 example.
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

#
# The default rule, which causes the qs_ek-lm3s2965 example to be built.
#
all: ${COMPILER}
all: ${COMPILER}/qs_ek-lm3s2965.axf

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
# Rules for building the qs_ek-lm3s2965 example.
#
${COMPILER}/qs_ek-lm3s2965.axf: ${COMPILER}/audio.o
${COMPILER}/qs_ek-lm3s2965.axf: ${COMPILER}/can_net.o
${COMPILER}/qs_ek-lm3s2965.axf: ${COMPILER}/game.o
${COMPILER}/qs_ek-lm3s2965.axf: ${COMPILER}/images.o
${COMPILER}/qs_ek-lm3s2965.axf: ${COMPILER}/osram128x64x4.o
${COMPILER}/qs_ek-lm3s2965.axf: ${COMPILER}/qs_ek-lm3s2965.o
${COMPILER}/qs_ek-lm3s2965.axf: ${COMPILER}/random.o
${COMPILER}/qs_ek-lm3s2965.axf: ${COMPILER}/screen_saver.o
${COMPILER}/qs_ek-lm3s2965.axf: ${COMPILER}/sounds.o
${COMPILER}/qs_ek-lm3s2965.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/qs_ek-lm3s2965.axf: ../../../src/${COMPILER}/libdriver.a
SCATTERgcc_qs_ek-lm3s2965=../../../gcc/standalone.ld
SCATTERewarm_qs_ek-lm3s2965=../../../ewarm/standalone.xcl
SCATTERsourcerygxx_qs_ek-lm3s2965=lm3s2965-rom.ld
ENTRY_qs_ek-lm3s2965=ResetISR
LDFLAGSsourcerygxx_qs_ek-lm3s2965=-T qs_ek-lm3s2965_sourcerygxx.ld

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
