//===-- Module.cpp - Implement the Module class ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Module class for the VMCore library.
//
//===----------------------------------------------------------------------===//

#include "llvm/Module.h"
#include "llvm/InstrTypes.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/LeakDetector.h"
#include "SymbolTableListTraitsImpl.h"
#include "llvm/TypeSymbolTable.h"
#include <algorithm>
#include <cstdarg>
#include <cstdlib>
using namespace llvm;

//===----------------------------------------------------------------------===//
// Methods to implement the globals and functions lists.
//

GlobalVariable *ilist_traits<GlobalVariable>::createSentinel() {
  GlobalVariable *Ret = new GlobalVariable(getGlobalContext(), 
                                           Type::getInt32Ty(getGlobalContext()),
                                           false, GlobalValue::ExternalLinkage);
  // This should not be garbage monitored.
  LeakDetector::removeGarbageObject(Ret);
  return Ret;
}
GlobalAlias *ilist_traits<GlobalAlias>::createSentinel() {
  GlobalAlias *Ret = new GlobalAlias(Type::getInt32Ty(getGlobalContext()),
                                     GlobalValue::ExternalLinkage);
  // This should not be garbage monitored.
  LeakDetector::removeGarbageObject(Ret);
  return Ret;
}

// Explicit instantiations of SymbolTableListTraits since some of the methods
// are not in the public header file.
template class SymbolTableListTraits<GlobalVariable, Module>;
template class SymbolTableListTraits<Function, Module>;
template class SymbolTableListTraits<GlobalAlias, Module>;

//===----------------------------------------------------------------------===//
// Primitive Module methods.
//

Module::Module(const StringRef &MID, LLVMContext& C)
  : Context(C), ModuleID(MID), DataLayout("")  {
  ValSymTab = new ValueSymbolTable();
  TypeSymTab = new TypeSymbolTable();
}

Module::~Module() {
  dropAllReferences();
  GlobalList.clear();
  FunctionList.clear();
  AliasList.clear();
  LibraryList.clear();
  NamedMDList.clear();
  delete ValSymTab;
  delete TypeSymTab;
}

/// Target endian information...
Module::Endianness Module::getEndianness() const {
  std::string temp = DataLayout;
  Module::Endianness ret = AnyEndianness;
  
  while (!temp.empty()) {
    std::string token = getToken(temp, "-");
    
    if (token[0] == 'e') {
      ret = LittleEndian;
    } else if (token[0] == 'E') {
      ret = BigEndian;
    }
  }
  
  return ret;
}

/// Target Pointer Size information...
Module::PointerSize Module::getPointerSize() const {
  std::string temp = DataLayout;
  Module::PointerSize ret = AnyPointerSize;
  
  while (!temp.empty()) {
    std::string token = getToken(temp, "-");
    char signal = getToken(token, ":")[0];
    
    if (signal == 'p') {
      int size = atoi(getToken(token, ":").c_str());
      if (size == 32)
        ret = Pointer32;
      else if (size == 64)
        ret = Pointer64;
    }
  }
  
  return ret;
}

/// getNamedValue - Return the first global value in the module with
/// the specified name, of arbitrary type.  This method returns null
/// if a global with the specified name is not found.
GlobalValue *Module::getNamedValue(const StringRef &Name) const {
  return cast_or_null<GlobalValue>(getValueSymbolTable().lookup(Name));
}

//===----------------------------------------------------------------------===//
// Methods for easy access to the functions in the module.
//

// getOrInsertFunction - Look up the specified function in the module symbol
// table.  If it does not exist, add a prototype for the function and return
// it.  This is nice because it allows most passes to get away with not handling
// the symbol table directly for this common task.
//
Constant *Module::getOrInsertFunction(const StringRef &Name,
                                      const FunctionType *Ty,
                                      AttrListPtr AttributeList) {
  // See if we have a definition for the specified function already.
  GlobalValue *F = getNamedValue(Name);
  if (F == 0) {
    // Nope, add it
    Function *New = Function::Create(Ty, GlobalVariable::ExternalLinkage, Name);
    if (!New->isIntrinsic())       // Intrinsics get attrs set on construction
      New->setAttributes(AttributeList);
    FunctionList.push_back(New);
    return New;                    // Return the new prototype.
  }

  // Okay, the function exists.  Does it have externally visible linkage?
  if (F->hasLocalLinkage()) {
    // Clear the function's name.
    F->setName("");
    // Retry, now there won't be a conflict.
    Constant *NewF = getOrInsertFunction(Name, Ty);
    F->setName(Name);
    return NewF;
  }

  // If the function exists but has the wrong type, return a bitcast to the
  // right type.
  if (F->getType() != PointerType::getUnqual(Ty))
    return ConstantExpr::getBitCast(F, PointerType::getUnqual(Ty));
  
  // Otherwise, we just found the existing function or a prototype.
  return F;  
}

Constant *Module::getOrInsertTargetIntrinsic(const StringRef &Name,
                                             const FunctionType *Ty,
                                             AttrListPtr AttributeList) {
  // See if we have a definition for the specified function already.
  GlobalValue *F = getNamedValue(Name);
  if (F == 0) {
    // Nope, add it
    Function *New = Function::Create(Ty, GlobalVariable::ExternalLinkage, Name);
    New->setAttributes(AttributeList);
    FunctionList.push_back(New);
    return New; // Return the new prototype.
  }

  // Otherwise, we just found the existing function or a prototype.
  return F;  
}

Constant *Module::getOrInsertFunction(const StringRef &Name,
                                      const FunctionType *Ty) {
  AttrListPtr AttributeList = AttrListPtr::get((AttributeWithIndex *)0, 0);
  return getOrInsertFunction(Name, Ty, AttributeList);
}

// getOrInsertFunction - Look up the specified function in the module symbol
// table.  If it does not exist, add a prototype for the function and return it.
// This version of the method takes a null terminated list of function
// arguments, which makes it easier for clients to use.
//
Constant *Module::getOrInsertFunction(const StringRef &Name,
                                      AttrListPtr AttributeList,
                                      const Type *RetTy, ...) {
  va_list Args;
  va_start(Args, RetTy);

  // Build the list of argument types...
  std::vector<const Type*> ArgTys;
  while (const Type *ArgTy = va_arg(Args, const Type*))
    ArgTys.push_back(ArgTy);

  va_end(Args);

  // Build the function type and chain to the other getOrInsertFunction...
  return getOrInsertFunction(Name,
                             FunctionType::get(RetTy, ArgTys, false),
                             AttributeList);
}

Constant *Module::getOrInsertFunction(const StringRef &Name,
                                      const Type *RetTy, ...) {
  va_list Args;
  va_start(Args, RetTy);

  // Build the list of argument types...
  std::vector<const Type*> ArgTys;
  while (const Type *ArgTy = va_arg(Args, const Type*))
    ArgTys.push_back(ArgTy);

  va_end(Args);

  // Build the function type and chain to the other getOrInsertFunction...
  return getOrInsertFunction(Name, 
                             FunctionType::get(RetTy, ArgTys, false),
                             AttrListPtr::get((AttributeWithIndex *)0, 0));
}

// getFunction - Look up the specified function in the module symbol table.
// If it does not exist, return null.
//
Function *Module::getFunction(const StringRef &Name) const {
  return dyn_cast_or_null<Function>(getNamedValue(Name));
}

//===----------------------------------------------------------------------===//
// Methods for easy access to the global variables in the module.
//

/// getGlobalVariable - Look up the specified global variable in the module
/// symbol table.  If it does not exist, return null.  The type argument
/// should be the underlying type of the global, i.e., it should not have
/// the top-level PointerType, which represents the address of the global.
/// If AllowLocal is set to true, this function will return types that
/// have an local. By default, these types are not returned.
///
GlobalVariable *Module::getGlobalVariable(const StringRef &Name,
                                          bool AllowLocal) const {
  if (GlobalVariable *Result = 
      dyn_cast_or_null<GlobalVariable>(getNamedValue(Name)))
    if (AllowLocal || !Result->hasLocalLinkage())
      return Result;
  return 0;
}

/// getOrInsertGlobal - Look up the specified global in the module symbol table.
///   1. If it does not exist, add a declaration of the global and return it.
///   2. Else, the global exists but has the wrong type: return the function
///      with a constantexpr cast to the right type.
///   3. Finally, if the existing global is the correct delclaration, return the
///      existing global.
Constant *Module::getOrInsertGlobal(const StringRef &Name, const Type *Ty) {
  // See if we have a definition for the specified global already.
  GlobalVariable *GV = dyn_cast_or_null<GlobalVariable>(getNamedValue(Name));
  if (GV == 0) {
    // Nope, add it
    GlobalVariable *New =
      new GlobalVariable(*this, Ty, false, GlobalVariable::ExternalLinkage,
                         0, Name);
     return New;                    // Return the new declaration.
  }

  // If the variable exists but has the wrong type, return a bitcast to the
  // right type.
  if (GV->getType() != PointerType::getUnqual(Ty))
    return ConstantExpr::getBitCast(GV, PointerType::getUnqual(Ty));
  
  // Otherwise, we just found the existing function or a prototype.
  return GV;
}

//===----------------------------------------------------------------------===//
// Methods for easy access to the global variables in the module.
//

// getNamedAlias - Look up the specified global in the module symbol table.
// If it does not exist, return null.
//
GlobalAlias *Module::getNamedAlias(const StringRef &Name) const {
  return dyn_cast_or_null<GlobalAlias>(getNamedValue(Name));
}

/// getNamedMetadata - Return the first NamedMDNode in the module with the
/// specified name. This method returns null if a NamedMDNode with the 
//// specified name is not found.
NamedMDNode *Module::getNamedMetadata(const StringRef &Name) const {
  return dyn_cast_or_null<NamedMDNode>(getValueSymbolTable().lookup(Name));
}

/// getOrInsertNamedMetadata - Return the first named MDNode in the module 
/// with the specified name. This method returns a new NamedMDNode if a 
/// NamedMDNode with the specified name is not found.
NamedMDNode *Module::getOrInsertNamedMetadata(const StringRef &Name) {
  NamedMDNode *NMD =
    dyn_cast_or_null<NamedMDNode>(getValueSymbolTable().lookup(Name));
  if (!NMD)
    NMD = NamedMDNode::Create(getContext(), Name, NULL, 0, this);
  return NMD;
}

//===----------------------------------------------------------------------===//
// Methods for easy access to the types in the module.
//


// addTypeName - Insert an entry in the symbol table mapping Str to Type.  If
// there is already an entry for this name, true is returned and the symbol
// table is not modified.
//
bool Module::addTypeName(const StringRef &Name, const Type *Ty) {
  TypeSymbolTable &ST = getTypeSymbolTable();

  if (ST.lookup(Name)) return true;  // Already in symtab...

  // Not in symbol table?  Set the name with the Symtab as an argument so the
  // type knows what to update...
  ST.insert(Name, Ty);

  return false;
}

/// getTypeByName - Return the type with the specified name in this module, or
/// null if there is none by that name.
const Type *Module::getTypeByName(const StringRef &Name) const {
  const TypeSymbolTable &ST = getTypeSymbolTable();
  return cast_or_null<Type>(ST.lookup(Name));
}

// getTypeName - If there is at least one entry in the symbol table for the
// specified type, return it.
//
std::string Module::getTypeName(const Type *Ty) const {
  const TypeSymbolTable &ST = getTypeSymbolTable();

  TypeSymbolTable::const_iterator TI = ST.begin();
  TypeSymbolTable::const_iterator TE = ST.end();
  if ( TI == TE ) return ""; // No names for types

  while (TI != TE && TI->second != Ty)
    ++TI;

  if (TI != TE)  // Must have found an entry!
    return TI->first;
  return "";     // Must not have found anything...
}

//===----------------------------------------------------------------------===//
// Other module related stuff.
//


// dropAllReferences() - This function causes all the subelementss to "let go"
// of all references that they are maintaining.  This allows one to 'delete' a
// whole module at a time, even though there may be circular references... first
// all references are dropped, and all use counts go to zero.  Then everything
// is deleted for real.  Note that no operations are valid on an object that
// has "dropped all references", except operator delete.
//
void Module::dropAllReferences() {
  for(Module::iterator I = begin(), E = end(); I != E; ++I)
    I->dropAllReferences();

  for(Module::global_iterator I = global_begin(), E = global_end(); I != E; ++I)
    I->dropAllReferences();

  for(Module::alias_iterator I = alias_begin(), E = alias_end(); I != E; ++I)
    I->dropAllReferences();
}

void Module::addLibrary(const StringRef& Lib) {
  for (Module::lib_iterator I = lib_begin(), E = lib_end(); I != E; ++I)
    if (*I == Lib)
      return;
  LibraryList.push_back(Lib);
}

void Module::removeLibrary(const StringRef& Lib) {
  LibraryListType::iterator I = LibraryList.begin();
  LibraryListType::iterator E = LibraryList.end();
  for (;I != E; ++I)
    if (*I == Lib) {
      LibraryList.erase(I);
      return;
    }
}
