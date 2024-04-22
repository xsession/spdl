#******************************************************************************
#
# Makefile - Rules for building the Sample Ethernet Application using lwIP.
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
PART=LM3S8962

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
VPATH+=../../../third_party/lwip-1.2.0/src/core
VPATH+=../../../third_party/lwip-1.2.0/src/core/ipv4
VPATH+=../../../third_party/lwip-1.2.0/src/netif
VPATH+=../../../utils
VPATH+=../../../third_party/lwip-1.2.0/apps/httpd
VPATH+=../../../third_party/fatfs/src
VPATH+=../../../third_party/fatfs/port

#
# Where to find header files that do not live in the source directory.
#
IPATH=.
IPATH+=../../../third_party/lwip-1.2.0/src/include
IPATH+=../../../third_party/lwip-1.2.0/src/include/ipv4
IPATH+=../../../third_party/lwip-1.2.0/port
IPATH+=../../../third_party/lwip-1.2.0/apps/httpd
IPATH+=../../../third_party/fatfs/src

#
# The default rule, which causes the Sample Ethernet Application using lwIP to be built.
#
all: ${COMPILER}
all: ${COMPILER}/enet_lwip.axf

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
# Rules for building the Sample Ethernet Application using lwIP.
#
${COMPILER}/enet_lwip.axf: ${COMPILER}/${DEBUGGER}.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/dhcp.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/enet_lwip.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/etharp.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/ff.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/httpd.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/icmp.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/inet.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/ip.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/ip_addr.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/lmi_fs.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/luminaryif.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/mem.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/memp.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/mmc-ek-lm3s8962.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/netif.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/pbuf.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/raw.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/rit128x96x4.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/stats.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/sys.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/tcp.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/tcp_in.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/tcp_out.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/udp.o
${COMPILER}/enet_lwip.axf: ${COMPILER}/ustdlib.o
${COMPILER}/enet_lwip.axf: ../../../src/${COMPILER}/libdriver.a
SCATTERgcc_enet_lwip=../../../gcc/standalone.ld
SCATTERewarm_enet_lwip=../../../ewarm/standalone.xcl
SCATTERsourcerygxx_enet_lwip=lm3s8962-rom.ld
ENTRY_enet_lwip=ResetISR
LDFLAGSsourcerygxx_enet_lwip=-T enet_lwip_sourcerygxx.ld

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
