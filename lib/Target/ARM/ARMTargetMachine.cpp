//===-- ARMTargetMachine.cpp - Define TargetMachine for ARM ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "ARMTargetMachine.h"
#include "ARMTargetAsmInfo.h"
#include "ARMFrameInfo.h"
#include "ARM.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetRegistry.h"
using namespace llvm;

static cl::opt<bool> DisableLdStOpti("disable-arm-loadstore-opti", cl::Hidden,
                              cl::desc("Disable load store optimization pass"));
static cl::opt<bool> DisableIfConversion("disable-arm-if-conversion",cl::Hidden,
                              cl::desc("Disable if-conversion pass"));

static const TargetAsmInfo *createTargetAsmInfo(const Target &T,
                                                const StringRef &TT) {
  Triple TheTriple(TT);
  switch (TheTriple.getOS()) {
  case Triple::Darwin:
    return new ARMDarwinTargetAsmInfo();
  default:
    return new ARMELFTargetAsmInfo();
  }
}


extern "C" void LLVMInitializeARMTarget() {
  // Register the target.
  RegisterTargetMachine<ARMTargetMachine> X(TheARMTarget);
  RegisterTargetMachine<ThumbTargetMachine> Y(TheThumbTarget);
  
  // Register the target asm info.
  RegisterAsmInfoFn A(TheARMTarget, createTargetAsmInfo);
  RegisterAsmInfoFn B(TheThumbTarget, createTargetAsmInfo);
}

/// TargetMachine ctor - Create an ARM architecture model.
///
ARMBaseTargetMachine::ARMBaseTargetMachine(const Target &T,
                                           const std::string &TT,
                                           const std::string &FS,
                                           bool isThumb)
  : LLVMTargetMachine(T, TT),
    Subtarget(TT, FS, isThumb),
    FrameInfo(Subtarget),
    JITInfo(),
    InstrItins(Subtarget.getInstrItineraryData()) {
  DefRelocModel = getRelocationModel();
}

ARMTargetMachine::ARMTargetMachine(const Target &T, const std::string &TT,
                                   const std::string &FS)
  : ARMBaseTargetMachine(T, TT, FS, false), InstrInfo(Subtarget),
    DataLayout(Subtarget.isAPCS_ABI() ?
               std::string("e-p:32:32-f64:32:32-i64:32:32") :
               std::string("e-p:32:32-f64:64:64-i64:64:64")),
    TLInfo(*this) {
}

ThumbTargetMachine::ThumbTargetMachine(const Target &T, const std::string &TT,
                                       const std::string &FS)
  : ARMBaseTargetMachine(T, TT, FS, true),
    InstrInfo(Subtarget.hasThumb2()
              ? ((ARMBaseInstrInfo*)new Thumb2InstrInfo(Subtarget))
              : ((ARMBaseInstrInfo*)new Thumb1InstrInfo(Subtarget))),
    DataLayout(Subtarget.isAPCS_ABI() ?
               std::string("e-p:32:32-f64:32:32-i64:32:32-"
                           "i16:16:32-i8:8:32-i1:8:32-a:0:32") :
               std::string("e-p:32:32-f64:64:64-i64:64:64-"
                           "i16:16:32-i8:8:32-i1:8:32-a:0:32")),
    TLInfo(*this) {
}



// Pass Pipeline Configuration
bool ARMBaseTargetMachine::addInstSelector(PassManagerBase &PM,
                                           CodeGenOpt::Level OptLevel) {
  PM.add(createARMISelDag(*this));
  return false;
}

bool ARMBaseTargetMachine::addPreRegAlloc(PassManagerBase &PM,
                                          CodeGenOpt::Level OptLevel) {
  if (Subtarget.hasNEON())
    PM.add(createNEONPreAllocPass());

  // FIXME: temporarily disabling load / store optimization pass for Thumb mode.
  if (OptLevel != CodeGenOpt::None && !DisableLdStOpti && !Subtarget.isThumb())
    PM.add(createARMLoadStoreOptimizationPass(true));
  return true;
}

bool ARMBaseTargetMachine::addPreEmitPass(PassManagerBase &PM,
                                          CodeGenOpt::Level OptLevel) {
  // FIXME: temporarily disabling load / store optimization pass for Thumb1 mode.
  if (OptLevel != CodeGenOpt::None && !DisableLdStOpti &&
      !Subtarget.isThumb1Only())
    PM.add(createARMLoadStoreOptimizationPass());

  if (OptLevel != CodeGenOpt::None &&
      !DisableIfConversion && !Subtarget.isThumb1Only())
    PM.add(createIfConverterPass());

  if (Subtarget.isThumb2()) {
    PM.add(createThumb2ITBlockPass());
    PM.add(createThumb2SizeReductionPass());
  }

  PM.add(createARMConstantIslandPass());
  return true;
}

bool ARMBaseTargetMachine::addCodeEmitter(PassManagerBase &PM,
                                          CodeGenOpt::Level OptLevel,
                                          MachineCodeEmitter &MCE) {
  // FIXME: Move this to TargetJITInfo!
  if (DefRelocModel == Reloc::Default)
    setRelocationModel(Reloc::Static);

  // Machine code emitter pass for ARM.
  PM.add(createARMCodeEmitterPass(*this, MCE));
  return false;
}

bool ARMBaseTargetMachine::addCodeEmitter(PassManagerBase &PM,
                                          CodeGenOpt::Level OptLevel,
                                          JITCodeEmitter &JCE) {
  // FIXME: Move this to TargetJITInfo!
  if (DefRelocModel == Reloc::Default)
    setRelocationModel(Reloc::Static);

  // Machine code emitter pass for ARM.
  PM.add(createARMJITCodeEmitterPass(*this, JCE));
  return false;
}

bool ARMBaseTargetMachine::addCodeEmitter(PassManagerBase &PM,
                                          CodeGenOpt::Level OptLevel,
                                          ObjectCodeEmitter &OCE) {
  // FIXME: Move this to TargetJITInfo!
  if (DefRelocModel == Reloc::Default)
    setRelocationModel(Reloc::Static);

  // Machine code emitter pass for ARM.
  PM.add(createARMObjectCodeEmitterPass(*this, OCE));
  return false;
}

bool ARMBaseTargetMachine::addSimpleCodeEmitter(PassManagerBase &PM,
                                                CodeGenOpt::Level OptLevel,
                                                MachineCodeEmitter &MCE) {
  // Machine code emitter pass for ARM.
  PM.add(createARMCodeEmitterPass(*this, MCE));
  return false;
}

bool ARMBaseTargetMachine::addSimpleCodeEmitter(PassManagerBase &PM,
                                                CodeGenOpt::Level OptLevel,
                                                JITCodeEmitter &JCE) {
  // Machine code emitter pass for ARM.
  PM.add(createARMJITCodeEmitterPass(*this, JCE));
  return false;
}

bool ARMBaseTargetMachine::addSimpleCodeEmitter(PassManagerBase &PM,
                                            CodeGenOpt::Level OptLevel,
                                            ObjectCodeEmitter &OCE) {
  // Machine code emitter pass for ARM.
  PM.add(createARMObjectCodeEmitterPass(*this, OCE));
  return false;
}

