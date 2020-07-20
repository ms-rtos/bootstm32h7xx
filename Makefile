#*********************************************************************************************************
#
#                                 ���������Ϣ�������޹�˾
#
#                                   ΢�Ͱ�ȫʵʱ����ϵͳ
#
#                                       MS-RTOS(TM)
#
#                               Copyright All Rights Reserved
#
#--------------�ļ���Ϣ--------------------------------------------------------------------------------
#
# ��   ��   ��: Makefile
#
# ��   ��   ��: IoT Studio
#
# �ļ���������: 2020 �� 05 �� 22 ��
#
# ��        ��: ���ļ��� IoT Studio ���ɣ��������� Makefile ���ܣ������ֶ��޸�
#*********************************************************************************************************

#*********************************************************************************************************
# Include config.mk
#*********************************************************************************************************
CONFIG_MK_EXIST = $(shell if [ -f ../config.mk ]; then echo exist; else echo notexist; fi;)
ifeq ($(CONFIG_MK_EXIST), exist)
include ../config.mk
else
CONFIG_MK_EXIST = $(shell if [ -f config.mk ]; then echo exist; else echo notexist; fi;)
ifeq ($(CONFIG_MK_EXIST), exist)
include config.mk
else
CONFIG_MK_EXIST =
endif
endif

#*********************************************************************************************************
# Include MS-RTOS base config.mk
#*********************************************************************************************************
EMPTY =
SPACE = $(EMPTY) $(EMPTY)

MSRTOS_BASE_PATH_BAK := $(MSRTOS_BASE_PATH)
TOOLCHAIN_PREFIX_BAK := $(TOOLCHAIN_PREFIX)
DEBUG_LEVEL_BAK      := $(DEBUG_LEVEL)
CPU_TYPE_BAK         := $(CPU_TYPE)
FPU_TYPE_BAK         := $(FPU_TYPE)

MSRTOS_BASE_CONFIGMK = $(subst $(SPACE),\ ,$(MSRTOS_BASE_PATH))/config.mk
include $(MSRTOS_BASE_CONFIGMK)

MSRTOS_BASE_PATH := $(MSRTOS_BASE_PATH_BAK)
DEBUG_LEVEL      := $(DEBUG_LEVEL_BAK)

ifneq ($(TOOLCHAIN_PREFIX_BAK),)
TOOLCHAIN_PREFIX := $(TOOLCHAIN_PREFIX_BAK)
endif

ifneq ($(CPU_TYPE_BAK),)
CPU_TYPE := $(CPU_TYPE_BAK)
endif

ifneq ($(FPU_TYPE_BAK),)
FPU_TYPE := $(FPU_TYPE_BAK)
endif

#*********************************************************************************************************
# Include header.mk
#*********************************************************************************************************
MKTEMP = $(subst $(SPACE),\ ,$(MSRTOS_BASE_PATH))/libmsrtos/src/mktemp

include $(MKTEMP)/header.mk

#*********************************************************************************************************
# Include targets makefiles
#*********************************************************************************************************
include bootstm32h7xx.mk

#*********************************************************************************************************
# Include end.mk
#*********************************************************************************************************
include $(END_MK)

#*********************************************************************************************************
# End
#*********************************************************************************************************
