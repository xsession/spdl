#******************************************************************************
#
# Makefile - Rules for building the ek-lm3s8962 examples.
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
# A list of the directories containing the examples.
#
DIRS=bitband       \
     blinky        \
     boot_demo1    \
     boot_demo2    \
     can_device_led \
     can_device_qs \
     enet_lwip     \
     enet_ptpd     \
     enet_uip      \
     gpio_jtag     \
     graphics      \
     hello         \
     interrupts    \
     mpu_fault     \
     pwmgen        \
     qs_ek-lm3s8962 \
     sd_card       \
     timers        \
     uart_echo     \
     watchdog

#
# The default rule, which causes the examples to be built.
#
all:
	@for i in ${DIRS};  \
	 do                 \
	     make -C $${i}; \
	 done

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${wildcard *~} __dummy__
	@-for i in ${DIRS};        \
	  do                       \
	      make -C $${i} clean; \
	  done
