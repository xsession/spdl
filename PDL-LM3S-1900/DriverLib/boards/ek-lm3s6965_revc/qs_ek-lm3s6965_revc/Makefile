#******************************************************************************
#
# Makefile - Rules for building the qs_ek-lm3s6965_revc example.
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
VPATH+=../../../third_party/lwip-1.2.0/port
VPATH+=../../../third_party/lwip-1.2.0/apps/httpd
VPATH+=../../../third_party/lwip-1.2.0/src/core
VPATH+=../../../third_party/lwip-1.2.0/src/core/ipv4
VPATH+=../../../third_party/lwip-1.2.0/src/netif

#
# Where to find header files that do not live in the source directory.
#
IPATH=.
IPATH+=../../../third_party/lwip-1.2.0/src/include
IPATH+=../../../third_party/lwip-1.2.0/src/include/ipv4
IPATH+=../../../third_party/lwip-1.2.0/port
IPATH+=../../../third_party/lwip-1.2.0/apps/httpd

#
# The default rule, which causes the qs_ek-lm3s6965_revc example to be built.
#
all: ${COMPILER}
all: ${COMPILER}/qs_ek-lm3s6965_revc.axf

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
# Rules for building the qs_ek-lm3s6965_revc example.
#
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/audio.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/dhcp.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/enet.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/etharp.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/fs_qs.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/game.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/httpd.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/icmp.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/images.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/inet.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/ip.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/ip_addr.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/luminaryif.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/mem.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/memp.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/netif.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/pbuf.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/qs_ek-lm3s6965_revc.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/random.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/raw.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/rit128x96x4.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/screen_saver.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/sounds.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/stats.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/sys.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/tcp.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/tcp_in.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/tcp_out.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ${COMPILER}/udp.o
${COMPILER}/qs_ek-lm3s6965_revc.axf: ../../../src/${COMPILER}/libdriver.a
SCATTERgcc_qs_ek-lm3s6965_revc=../../../gcc/standalone.ld
SCATTERewarm_qs_ek-lm3s6965_revc=../../../ewarm/standalone.xcl
SCATTERsourcerygxx_qs_ek-lm3s6965_revc=lm3s6965-rom.ld
ENTRY_qs_ek-lm3s6965_revc=ResetISR
LDFLAGSsourcerygxx_qs_ek-lm3s6965_revc=-T qs_ek-lm3s6965_revc_sourcerygxx.ld

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
