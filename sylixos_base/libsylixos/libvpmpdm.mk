#*********************************************************************************************************
#
#                                    中国软件开源组织
#
#                                   嵌入式实时操作系统
#
#                                SylixOS(TM)  LW : long wing
#
#                               Copyright All Rights Reserved
#
#--------------文件信息--------------------------------------------------------------------------------
#
# 文   件   名: libvpmpdm.mk
#
# 创   建   人: RealEvo-IDE
#
# 文件创建日期: 2016 年 10 月 08 日
#
# 描        述: 本文件由 RealEvo-IDE 生成，用于配置 Makefile 功能，请勿手动修改
#*********************************************************************************************************

#*********************************************************************************************************
# Clear setting
#*********************************************************************************************************
include $(CLEAR_VARS_MK)

#*********************************************************************************************************
# Target
#*********************************************************************************************************
LOCAL_TARGET_NAME := libvpmpdm.so

#*********************************************************************************************************
# Source list
#*********************************************************************************************************
LOCAL_SRCS := \
SylixOS/vpmpdm/cfloat/backtrace/backtrace.c \
SylixOS/vpmpdm/cfloat/float/float.c \
SylixOS/vpmpdm/cfloat/iniparser/dictionary.c \
SylixOS/vpmpdm/cfloat/iniparser/iniparser.c \
SylixOS/vpmpdm/cfloat/stdio/asprintf.c \
SylixOS/vpmpdm/cfloat/stdio/cvtfloat.c \
SylixOS/vpmpdm/cfloat/stdio/fdprintf.c \
SylixOS/vpmpdm/cfloat/stdio/fdscanf.c \
SylixOS/vpmpdm/cfloat/stdio/fprintf.c \
SylixOS/vpmpdm/cfloat/stdio/fscanf.c \
SylixOS/vpmpdm/cfloat/stdio/gets.c \
SylixOS/vpmpdm/cfloat/stdio/printf.c \
SylixOS/vpmpdm/cfloat/stdio/puts.c \
SylixOS/vpmpdm/cfloat/stdio/scanf.c \
SylixOS/vpmpdm/cfloat/stdio/snprintf.c \
SylixOS/vpmpdm/cfloat/stdio/sprintf.c \
SylixOS/vpmpdm/cfloat/stdio/sscanf.c \
SylixOS/vpmpdm/cfloat/stdio/vfprintf.c \
SylixOS/vpmpdm/cfloat/stdio/vfscanf.c \
SylixOS/vpmpdm/cfloat/stdio/vprintf.c \
SylixOS/vpmpdm/cfloat/stdio/vscanf.c \
SylixOS/vpmpdm/cfloat/stdio/vsnprintf.c \
SylixOS/vpmpdm/cfloat/stdio/vsprintf.c \
SylixOS/vpmpdm/cfloat/stdio/vsscanf.c \
SylixOS/vpmpdm/cfloat/stdlib/lib_rand.c \
SylixOS/vpmpdm/cfloat/stdlib/lib_search.c \
SylixOS/vpmpdm/cfloat/stdlib/lib_sort.c \
SylixOS/vpmpdm/cfloat/stdlib/lib_strto.c \
SylixOS/vpmpdm/cfloat/stdlib/lib_strtod.c \
SylixOS/vpmpdm/cfloat/time/lib_difftime.c \
SylixOS/vpmpdm/cfloat/wchar/wchar.c \
SylixOS/vpmpdm/cfloat/wchar/wcsdup.c \
SylixOS/vpmpdm/dlmalloc/dl_malloc.c \
SylixOS/vpmpdm/dlmalloc/dlmalloc.c \
SylixOS/vpmpdm/net/getifaddrs.c \
SylixOS/vpmpdm/net/if.c \
SylixOS/vpmpdm/tlsf/tlsf.c \
SylixOS/vpmpdm/vpmpdm_cpp.cpp \
SylixOS/vpmpdm/vpmpdm_lm.c \
SylixOS/vpmpdm/vpmpdm_start.c \
SylixOS/vpmpdm/vpmpdm.c

#*********************************************************************************************************
# ARM source
#*********************************************************************************************************
LOCAL_ARM_SRCS = \
SylixOS/vpmpdm/arch/arm/generic/memcmp.S \
SylixOS/vpmpdm/arch/arm/generic/memcpy.S \
SylixOS/vpmpdm/arch/arm/generic/memset.S \
SylixOS/vpmpdm/arch/arm/generic/strcmp.S \
SylixOS/vpmpdm/arch/arm/generic/strcpy.S \
SylixOS/vpmpdm/arch/arm/generic/strlen.c

ifeq ($(CPU_TYPE), cortex-a7) 
LOCAL_ARM_SRCS += \
SylixOS/vpmpdm/arch/arm/cortex-a7/memset.S \
SylixOS/vpmpdm/arch/arm/cortex-a7/memcpy.S

LOCAL_EXCLUDE_SRCS += \
SylixOS/vpmpdm/arch/arm/cortex-a15/memcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/memset.S \
SylixOS/vpmpdm/arch/arm/generic/memcpy.S \
SylixOS/vpmpdm/arch/arm/generic/memset.S
endif

ifeq ($(CPU_TYPE), cortex-a9) 
LOCAL_ARM_SRCS += \
SylixOS/vpmpdm/arch/arm/cortex-a9/memcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a9/memset.S \
SylixOS/vpmpdm/arch/arm/cortex-a9/stpcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a9/strcat.S \
SylixOS/vpmpdm/arch/arm/cortex-a9/strcmp.S \
SylixOS/vpmpdm/arch/arm/cortex-a9/strcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a9/strlen.S

LOCAL_EXCLUDE_SRCS += \
SylixOS/vpmpdm/arch/arm/cortex-a15/memset.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/memcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/stpcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/strcat.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/strcmp.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/strcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/strlen.S \
SylixOS/vpmpdm/arch/arm/generic/memcpy.S \
SylixOS/vpmpdm/arch/arm/generic/memset.S \
SylixOS/vpmpdm/arch/arm/generic/strcmp.S \
SylixOS/vpmpdm/arch/arm/generic/strcpy.S \
SylixOS/vpmpdm/arch/arm/generic/strlen.c
endif

ifeq ($(CPU_TYPE), cortex-a53) 
LOCAL_ARM_SRCS += \
SylixOS/vpmpdm/arch/arm/cortex-a53/memcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a7/memset.S

LOCAL_EXCLUDE_SRCS += \
SylixOS/vpmpdm/arch/arm/cortex-a15/memset.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/memcpy.S \
SylixOS/vpmpdm/arch/arm/generic/memcpy.S \
SylixOS/vpmpdm/arch/arm/generic/memset.S
endif

ifeq ($(CPU_TYPE), cortex-a73) 
LOCAL_ARM_SRCS += \
SylixOS/vpmpdm/arch/arm/cortex-a7/memset.S \
SylixOS/vpmpdm/arch/arm/denver/memcpy.S \
SylixOS/vpmpdm/arch/arm/krait/strcmp.S

LOCAL_EXCLUDE_SRCS += \
SylixOS/vpmpdm/arch/arm/cortex-a15/memset.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/memcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/strcmp.S \
SylixOS/vpmpdm/arch/arm/generic/memcpy.S \
SylixOS/vpmpdm/arch/arm/generic/memset.S \
SylixOS/vpmpdm/arch/arm/generic/strcmp.S
endif

ifneq (,$(findstring neon,$(FPU_TYPE)))
LOCAL_ARM_SRCS += \
SylixOS/vpmpdm/arch/arm/cortex-a15/memcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/memset.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/stpcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/strcat.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/strcmp.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/strcpy.S \
SylixOS/vpmpdm/arch/arm/cortex-a15/strlen.S \
SylixOS/vpmpdm/arch/arm/denver/memmove.S

LOCAL_EXCLUDE_SRCS += \
SylixOS/vpmpdm/arch/arm/generic/memcpy.S \
SylixOS/vpmpdm/arch/arm/generic/memset.S \
SylixOS/vpmpdm/arch/arm/generic/strcmp.S \
SylixOS/vpmpdm/arch/arm/generic/strcpy.S \
SylixOS/vpmpdm/arch/arm/generic/strlen.c
endif

#*********************************************************************************************************
# MIPS source
#*********************************************************************************************************
LOCAL_MIPS_SRCS = \
SylixOS/vpmpdm/arch/mips/memcmp.c \
SylixOS/vpmpdm/arch/mips/memcpy.S \
SylixOS/vpmpdm/arch/mips/memset.S \
SylixOS/vpmpdm/arch/mips/strcmp.S \
SylixOS/vpmpdm/arch/mips/strncmp.S \
SylixOS/vpmpdm/arch/mips/strlen.c \
SylixOS/vpmpdm/arch/mips/strnlen.c \
SylixOS/vpmpdm/arch/mips/strchr.c \
SylixOS/vpmpdm/arch/mips/strcpy.c \
SylixOS/vpmpdm/arch/mips/memchr.c \
SylixOS/vpmpdm/arch/mips/memmove.c

#*********************************************************************************************************
# MIPS64 source
#*********************************************************************************************************
LOCAL_MIPS64_SRCS = \
SylixOS/vpmpdm/arch/mips/memcmp.c \
SylixOS/vpmpdm/arch/mips/memcpy.S \
SylixOS/vpmpdm/arch/mips/memset.S \
SylixOS/vpmpdm/arch/mips/strcmp.S \
SylixOS/vpmpdm/arch/mips/strncmp.S \
SylixOS/vpmpdm/arch/mips/strlen.c \
SylixOS/vpmpdm/arch/mips/strnlen.c \
SylixOS/vpmpdm/arch/mips/strchr.c \
SylixOS/vpmpdm/arch/mips/strcpy.c \
SylixOS/vpmpdm/arch/mips/memchr.c \
SylixOS/vpmpdm/arch/mips/memmove.c

#*********************************************************************************************************
# PowerPC source
#*********************************************************************************************************
LOCAL_PPC_SRCS = \
SylixOS/vpmpdm/arch/ppc/stpcpy.S \
SylixOS/vpmpdm/arch/ppc/strchr.S \
SylixOS/vpmpdm/arch/ppc/strcmp.S \
SylixOS/vpmpdm/arch/ppc/strcpy.S \
SylixOS/vpmpdm/arch/ppc/strlen.S \
SylixOS/vpmpdm/arch/ppc/strncmp.S

#*********************************************************************************************************
# x86 source
#*********************************************************************************************************
LOCAL_X86_SRCS = \
SylixOS/vpmpdm/arch/x86/generic/memcmp.S \
SylixOS/vpmpdm/arch/x86/generic/strcmp.S \
SylixOS/vpmpdm/arch/x86/generic/strncmp.S \
SylixOS/vpmpdm/arch/x86/generic/strcat.S

ifeq ($(CPU_TYPE), $(filter $(CPU_TYPE),atom pentium))
LOCAL_X86_SRCS += \
SylixOS/vpmpdm/arch/x86/atom/sse2-memchr-atom.S \
SylixOS/vpmpdm/arch/x86/atom/sse2-memrchr-atom.S \
SylixOS/vpmpdm/arch/x86/atom/sse2-strchr-atom.S \
SylixOS/vpmpdm/arch/x86/atom/sse2-strnlen-atom.S \
SylixOS/vpmpdm/arch/x86/atom/sse2-strrchr-atom.S \
SylixOS/vpmpdm/arch/x86/atom/sse2-wcschr-atom.S \
SylixOS/vpmpdm/arch/x86/atom/sse2-wcsrchr-atom.S \
SylixOS/vpmpdm/arch/x86/atom/sse2-wcslen-atom.S \
SylixOS/vpmpdm/arch/x86/atom/sse2-wcscmp-atom.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-memcpy-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-memmove-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-memset-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-stpcpy-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-stpncpy-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-strcpy-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-strlen-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-strncpy-slm.S
endif

ifeq ($(CPU_TYPE), atom) 
LOCAL_X86_SRCS += \
SylixOS/vpmpdm/arch/x86/atom/sse2-memset-atom.S \
SylixOS/vpmpdm/arch/x86/atom/sse2-strlen-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-memcmp-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-memcpy-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-memmove-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-strcpy-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-strncpy-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-wmemcmp-atom.S

LOCAL_EXCLUDE_SRCS += \
SylixOS/vpmpdm/arch/x86/generic/memcmp.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-memcpy-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-memmove-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-memset-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-strcpy-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-strlen-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse2-strncpy-slm.S
endif

ifneq (,$(findstring ssse3,$(FPU_TYPE)))
LOCAL_X86_SRCS += \
SylixOS/vpmpdm/arch/x86/atom/ssse3-strncat-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-strlcat-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-strlcpy-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-strcat-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-strcmp-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-strncmp-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-wcscat-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-wcscpy-atom.S

LOCAL_EXCLUDE_SRCS += \
SylixOS/vpmpdm/arch/x86/generic/strcmp.S \
SylixOS/vpmpdm/arch/x86/generic/strncmp.S \
SylixOS/vpmpdm/arch/x86/generic/strcat.S
endif

ifneq (,$(findstring sse4,$(FPU_TYPE)))
LOCAL_X86_SRCS += \
SylixOS/vpmpdm/arch/x86/silvermont/sse4-memcmp-slm.S \
SylixOS/vpmpdm/arch/x86/silvermont/sse4-wmemcmp-slm.S

LOCAL_EXCLUDE_SRCS += \
SylixOS/vpmpdm/arch/x86/generic/memcmp.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-memcmp-atom.S \
SylixOS/vpmpdm/arch/x86/atom/ssse3-wmemcmp-atom.S
endif

#*********************************************************************************************************
# x86-64 source
#*********************************************************************************************************
LOCAL_X64_SRCS = \
SylixOS/vpmpdm/arch/x64/sse2-memcpy-slm.S \
SylixOS/vpmpdm/arch/x64/sse2-memmove-slm.S \
SylixOS/vpmpdm/arch/x64/sse2-memset-slm.S \
SylixOS/vpmpdm/arch/x64/sse2-stpcpy-slm.S \
SylixOS/vpmpdm/arch/x64/sse2-stpncpy-slm.S \
SylixOS/vpmpdm/arch/x64/sse2-strcat-slm.S \
SylixOS/vpmpdm/arch/x64/sse2-strcpy-slm.S \
SylixOS/vpmpdm/arch/x64/sse2-strlen-slm.S \
SylixOS/vpmpdm/arch/x64/sse2-strncat-slm.S \
SylixOS/vpmpdm/arch/x64/sse2-strncpy-slm.S

ifneq (,$(findstring ssse3,$(FPU_TYPE)))
LOCAL_X64_SRCS += \
SylixOS/vpmpdm/arch/x64/ssse3-strcmp-slm.S \
SylixOS/vpmpdm/arch/x64/ssse3-strncmp-slm.S
endif

ifneq (,$(findstring sse4,$(FPU_TYPE)))
LOCAL_X64_SRCS += \
SylixOS/vpmpdm/arch/x64/sse4-memcmp-slm.S
endif

#*********************************************************************************************************
# TI C6X DSP source
#*********************************************************************************************************
LOCAL_C6X_SRCS = \
SylixOS/vpmpdm/arch/c6x/alloca.c \
SylixOS/vpmpdm/arch/c6x/libc.c

ifeq ($(DEBUG_LEVEL), release)
LOCAL_C6X_SRCS += \
SylixOS/vpmpdm/arch/c6x/memcpy62.c \
SylixOS/vpmpdm/arch/c6x/memcpy64.asm
endif

#*********************************************************************************************************
# SPARC source
#*********************************************************************************************************
LOCAL_SPARC_SRCS = \
SylixOS/vpmpdm/arch/sparc/memchr.S \
SylixOS/vpmpdm/arch/sparc/memcpy.S \
SylixOS/vpmpdm/arch/sparc/memset.S \
SylixOS/vpmpdm/arch/sparc/stpcpy.S \
SylixOS/vpmpdm/arch/sparc/strcat.S \
SylixOS/vpmpdm/arch/sparc/strchr.S \
SylixOS/vpmpdm/arch/sparc/strcmp.S \
SylixOS/vpmpdm/arch/sparc/strcpy.S \
SylixOS/vpmpdm/arch/sparc/strlen.S \
SylixOS/vpmpdm/arch/sparc/strrchr.S

#*********************************************************************************************************
# Header file search path (eg. LOCAL_INC_PATH := -I"Your hearder files search path")
#*********************************************************************************************************
LOCAL_INC_PATH := -I"SylixOS/vpmpdm/arch" 
LOCAL_INC_PATH += -I"SylixOS/vpmpdm/arch/$(ARCH)"

#*********************************************************************************************************
# Pre-defined macro (eg. -DYOUR_MARCO=1)
#*********************************************************************************************************
LOCAL_DSYMBOL := 

#*********************************************************************************************************
# Depend library (eg. LOCAL_DEPEND_LIB := -la LOCAL_DEPEND_LIB_PATH := -L"Your library search path")
#*********************************************************************************************************
LOCAL_DEPEND_LIB      := 
LOCAL_DEPEND_LIB_PATH := 

#*********************************************************************************************************
# C++ config
#*********************************************************************************************************
LOCAL_USE_CXX        := no
LOCAL_USE_CXX_EXCEPT := no

#*********************************************************************************************************
# Code coverage config
#*********************************************************************************************************
LOCAL_USE_GCOV := no

include $(LIBRARY_MK)

#*********************************************************************************************************
# End
#*********************************************************************************************************
