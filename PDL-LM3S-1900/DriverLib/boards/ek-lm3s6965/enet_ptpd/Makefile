#******************************************************************************
#
# Makefile - Rules for building the Sample IEEE-1588 Application using Open Source PTPd with lwIP.
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
VPATH+=../../../third_party/lwip-1.2.0/src/core
VPATH+=../../../third_party/lwip-1.2.0/src/core/ipv4
VPATH+=../../../third_party/lwip-1.2.0/src/netif
VPATH+=../../../utils
VPATH+=../../../third_party/lwip-1.2.0/apps/httpd
VPATH+=../../../third_party/ptpd-1rc1/src
VPATH+=../../../third_party/ptpd-1rc1/src/dep-lmi

#
# Where to find header files that do not live in the source directory.
#
IPATH=.
IPATH+=../../../third_party/lwip-1.2.0/src/include
IPATH+=../../../third_party/lwip-1.2.0/src/include/ipv4
IPATH+=../../../third_party/lwip-1.2.0/port
IPATH+=../../../third_party/lwip-1.2.0/apps/httpd
IPATH+=../../../third_party/ptpd-1rc1/src
IPATH+=../../..

#
# The default rule, which causes the Sample IEEE-1588 Application using Open Source PTPd with lwIP to be built.
#
all: ${COMPILER}
all: ${COMPILER}/enet_ptpd.axf

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
# Rules for building the Sample IEEE-1588 Application using Open Source PTPd with lwIP.
#
${COMPILER}/enet_ptpd.axf: ${COMPILER}/${DEBUGGER}.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/arith.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/bmc.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/dhcp.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/enet_lwip.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/enet_ptpd.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/etharp.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/httpd.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/icmp.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/inet.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/ip.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/ip_addr.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/lmi_fs.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/luminaryif.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/mem.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/memp.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/netif.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/osram128x64x4.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/pbuf.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/protocol.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/ptpd_msg.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/ptpd_net.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/ptpd_servo.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/ptpd_timer.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/random.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/raw.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/stats.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/sys.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/tcp.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/tcp_in.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/tcp_out.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/udp.o
${COMPILER}/enet_ptpd.axf: ${COMPILER}/ustdlib.o
${COMPILER}/enet_ptpd.axf: ../../../src/${COMPILER}/libdriver.a
SCATTERgcc_enet_ptpd=../../../gcc/standalone.ld
SCATTERewarm_enet_ptpd=../../../ewarm/standalone.xcl
SCATTERsourcerygxx_enet_ptpd=lm3s6965-rom.ld
ENTRY_enet_ptpd=ResetISR
LDFLAGSsourcerygxx_enet_ptpd=-T enet_ptpd_sourcerygxx.ld

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
