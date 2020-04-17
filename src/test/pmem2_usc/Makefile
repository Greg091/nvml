# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2019-2020, Intel Corporation

#
# src/test/pmem2_persist/Makefile -- build pmem2_persist unit test
#
TOP = ../../..

vpath %.c $(TOP)/src/examples/libpmem2/unsafe_shutdowns

TARGET = pmem2_usc
LIBS = -lpmem2

INCS += -I$(TOP)/src/test/pmem2_usc

OBJS += unsafe_shutdowns.o\
	mocks_posix.o

include ../Makefile.inc
LDFLAGS += $(call extract_funcs, mocks_posix.c)
