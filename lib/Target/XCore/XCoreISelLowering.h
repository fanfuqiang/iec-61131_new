//===-- XCoreISelLowering.h - XCore DAG Lowering Interface ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that XCore uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef XCOREISELLOWERING_H
#define XCOREISELLOWERING_H

#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Target/TargetLowering.h"
#include "XCore.h"

namespace llvm {
  
  // Forward delcarations
  class XCoreSubtarget;
  class XCoreTargetMachine;
  
  namespace XCoreISD {
    enum NodeType {
      // Start the numbering where the builtin ops and target ops leave off.
      FIRST_NUMBER = ISD::BUILTIN_OP_END+XCore::INSTRUCTION_LIST_END,

      // Branch and link (call)
      BL,

      // pc relative address
      PCRelativeWrapper,

      // dp relative address
      DPRelativeWrapper,
      
      // cp relative address
      CPRelativeWrapper,
      
      // Store word to stack
      STWSP,

      // Corresponds to retsp instruction
      RETSP,
      
      // Corresponds to LADD instruction
      LADD,

      // Corresponds to LSUB instruction
      LSUB
    };
  }

  //===--------------------------------------------------------------------===//
  // TargetLowering Implementation
  //===--------------------------------------------------------------------===//
  class XCoreTargetLowering : public TargetLowering 
  {
  public:

    explicit XCoreTargetLowering(XCoreTargetMachine &TM);

    /// LowerOperation - Provide custom lowering hooks for some operations.
    virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG);

    /// ReplaceNodeResults - Replace the results of node with an illegal result
    /// type with new values built out of custom code.
    ///
    virtual void ReplaceNodeResults(SDNode *N, SmallVectorImpl<SDValue>&Results,
                                    SelectionDAG &DAG);

    /// getTargetNodeName - This method returns the name of a target specific 
    //  DAG node.
    virtual const char *getTargetNodeName(unsigned Opcode) const;
  
    virtual MachineBasicBlock *EmitInstrWithCustomInserter(MachineInstr *MI,
                                                  MachineBasicBlock *MBB) const;

    virtual bool isLegalAddressingMode(const AddrMode &AM,
                                       const Type *Ty) const;

    /// getFunctionAlignment - Return the Log2 alignment of this function.
    virtual unsigned getFunctionAlignment(const Function *F) const;

  private:
    const XCoreTargetMachine &TM;
    const XCoreSubtarget &Subtarget;
  
    // Lower Operand helpers
    SDValue LowerCCCArguments(SDValue Chain,
                              unsigned CallConv,
                              bool isVarArg,
                              const SmallVectorImpl<ISD::InputArg> &Ins,
                              DebugLoc dl, SelectionDAG &DAG,
                              SmallVectorImpl<SDValue> &InVals);
    SDValue LowerCCCCallTo(SDValue Chain, SDValue Callee,
                           unsigned CallConv, bool isVarArg,
                           bool isTailCall,
                           const SmallVectorImpl<ISD::OutputArg> &Outs,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           DebugLoc dl, SelectionDAG &DAG,
                           SmallVectorImpl<SDValue> &InVals);
    SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                            unsigned CallConv, bool isVarArg,
                            const SmallVectorImpl<ISD::InputArg> &Ins,
                            DebugLoc dl, SelectionDAG &DAG,
                            SmallVectorImpl<SDValue> &InVals);
    SDValue getReturnAddressFrameIndex(SelectionDAG &DAG);
    SDValue getGlobalAddressWrapper(SDValue GA, GlobalValue *GV,
                                    SelectionDAG &DAG);

    // Lower Operand specifics
    SDValue LowerLOAD(SDValue Op, SelectionDAG &DAG);
    SDValue LowerSTORE(SDValue Op, SelectionDAG &DAG);
    SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG);
    SDValue LowerGlobalTLSAddress(SDValue Op, SelectionDAG &DAG);
    SDValue LowerConstantPool(SDValue Op, SelectionDAG &DAG);
    SDValue LowerJumpTable(SDValue Op, SelectionDAG &DAG);
    SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG);
    SDValue LowerVAARG(SDValue Op, SelectionDAG &DAG);
    SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG);
    SDValue LowerFRAMEADDR(SDValue Op, SelectionDAG &DAG);
  
    // Inline asm support
    std::vector<unsigned>
    getRegClassForInlineAsmConstraint(const std::string &Constraint,
              EVT VT) const;
  
    // Expand specifics
    SDValue ExpandADDSUB(SDNode *Op, SelectionDAG &DAG);

    virtual SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const;

    virtual SDValue
      LowerFormalArguments(SDValue Chain,
                           unsigned CallConv,
                           bool isVarArg,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           DebugLoc dl, SelectionDAG &DAG,
                           SmallVectorImpl<SDValue> &InVals);

    virtual SDValue
      LowerCall(SDValue Chain, SDValue Callee,
                unsigned CallConv, bool isVarArg,
                bool isTailCall,
                const SmallVectorImpl<ISD::OutputArg> &Outs,
                const SmallVectorImpl<ISD::InputArg> &Ins,
                DebugLoc dl, SelectionDAG &DAG,
                SmallVectorImpl<SDValue> &InVals);

    virtual SDValue
      LowerReturn(SDValue Chain,
                  unsigned CallConv, bool isVarArg,
                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                  DebugLoc dl, SelectionDAG &DAG);
  };
}

#endif // XCOREISELLOWERING_H
