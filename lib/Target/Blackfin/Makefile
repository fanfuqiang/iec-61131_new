##===- lib/Target/Blackfin/Makefile ------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##
LEVEL = ../../..
LIBRARYNAME = LLVMBlackfinCodeGen
TARGET = Blackfin

# Make sure that tblgen is run, first thing.
BUILT_SOURCES = BlackfinGenRegisterInfo.h.inc BlackfinGenRegisterNames.inc \
                BlackfinGenRegisterInfo.inc BlackfinGenInstrNames.inc \
                BlackfinGenInstrInfo.inc BlackfinGenAsmWriter.inc \
                BlackfinGenDAGISel.inc BlackfinGenSubtarget.inc \
		BlackfinGenCallingConv.inc

DIRS = AsmPrinter TargetInfo

include $(LEVEL)/Makefile.common

