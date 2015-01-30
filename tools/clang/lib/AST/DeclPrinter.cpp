//===--- DeclPrinter.cpp - Printing implementation for Decl ASTs ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Decl::dump method, which pretty print the
// AST back out to C/Objective-C/C++/Objective-C++ code.
//
//===----------------------------------------------------------------------===//
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/Expr.h"
#include "clang/AST/PrettyPrinter.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Streams.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
using namespace clang;

namespace {
  class VISIBILITY_HIDDEN DeclPrinter : public DeclVisitor<DeclPrinter> {
    llvm::raw_ostream &Out;
    ASTContext &Context;
    PrintingPolicy Policy;
    unsigned Indentation;

    llvm::raw_ostream& Indent();
    void ProcessDeclGroup(llvm::SmallVectorImpl<Decl*>& Decls);

  public:
    DeclPrinter(llvm::raw_ostream &Out, ASTContext &Context, 
                const PrintingPolicy &Policy,
                unsigned Indentation = 0)
      : Out(Out), Context(Context), Policy(Policy), Indentation(Indentation) { }

    void VisitDeclContext(DeclContext *DC, bool Indent = true);

    void VisitTranslationUnitDecl(TranslationUnitDecl *D);
    void VisitTypedefDecl(TypedefDecl *D);
    void VisitEnumDecl(EnumDecl *D);
    void VisitRecordDecl(RecordDecl *D);
    void VisitEnumConstantDecl(EnumConstantDecl *D);
    void VisitFunctionDecl(FunctionDecl *D);
    void VisitFieldDecl(FieldDecl *D);
    void VisitVarDecl(VarDecl *D);
    void VisitParmVarDecl(ParmVarDecl *D);
    void VisitOriginalParmVarDecl(OriginalParmVarDecl *D);
    void VisitFileScopeAsmDecl(FileScopeAsmDecl *D);
    void VisitOverloadedFunctionDecl(OverloadedFunctionDecl *D);
    void VisitNamespaceDecl(NamespaceDecl *D);
    void VisitUsingDirectiveDecl(UsingDirectiveDecl *D);
    void VisitNamespaceAliasDecl(NamespaceAliasDecl *D);
    void VisitCXXRecordDecl(CXXRecordDecl *D);
    void VisitLinkageSpecDecl(LinkageSpecDecl *D);
    void VisitTemplateDecl(TemplateDecl *D);
    void VisitObjCMethodDecl(ObjCMethodDecl *D);
    void VisitObjCClassDecl(ObjCClassDecl *D);
    void VisitObjCImplementationDecl(ObjCImplementationDecl *D);
    void VisitObjCInterfaceDecl(ObjCInterfaceDecl *D);
    void VisitObjCForwardProtocolDecl(ObjCForwardProtocolDecl *D);
    void VisitObjCProtocolDecl(ObjCProtocolDecl *D);
    void VisitObjCCategoryImplDecl(ObjCCategoryImplDecl *D);
    void VisitObjCCategoryDecl(ObjCCategoryDecl *D);
    void VisitObjCCompatibleAliasDecl(ObjCCompatibleAliasDecl *D);
    void VisitObjCPropertyDecl(ObjCPropertyDecl *D);
    void VisitObjCPropertyImplDecl(ObjCPropertyImplDecl *D);
  };
}

void Decl::print(llvm::raw_ostream &Out, unsigned Indentation) {
  print(Out, getASTContext().PrintingPolicy, Indentation);
}

void Decl::print(llvm::raw_ostream &Out, const PrintingPolicy &Policy,
                 unsigned Indentation) {
  DeclPrinter Printer(Out, getASTContext(), Policy, Indentation);
  Printer.Visit(this);
}

static QualType GetBaseType(QualType T) {
  // FIXME: This should be on the Type class!
  QualType BaseType = T;
  while (!BaseType->isSpecifierType()) {
    if (isa<TypedefType>(BaseType))
      break;
    else if (const PointerType* PTy = BaseType->getAs<PointerType>())
      BaseType = PTy->getPointeeType();
    else if (const ArrayType* ATy = dyn_cast<ArrayType>(BaseType))
      BaseType = ATy->getElementType();
    else if (const FunctionType* FTy = BaseType->getAsFunctionType())
      BaseType = FTy->getResultType();
    else if (const VectorType *VTy = BaseType->getAsVectorType())
      BaseType = VTy->getElementType();
    else
      assert(0 && "Unknown declarator!");
  }
  return BaseType;
}

static QualType getDeclType(Decl* D) {
  if (TypedefDecl* TDD = dyn_cast<TypedefDecl>(D))
    return TDD->getUnderlyingType();
  if (ValueDecl* VD = dyn_cast<ValueDecl>(D))
    return VD->getType();
  return QualType();
}

void Decl::printGroup(Decl** Begin, unsigned NumDecls,
                      llvm::raw_ostream &Out, const PrintingPolicy &Policy,
                      unsigned Indentation) {
  if (NumDecls == 1) {
    (*Begin)->print(Out, Policy, Indentation);
    return;
  }

  Decl** End = Begin + NumDecls;
  TagDecl* TD = dyn_cast<TagDecl>(*Begin);
  if (TD)
    ++Begin;

  PrintingPolicy SubPolicy(Policy);
  if (TD && TD->isDefinition()) {
    TD->print(Out, Policy, Indentation);
    Out << " ";
    SubPolicy.SuppressTag = true;
  }

  bool isFirst = true;
  for ( ; Begin != End; ++Begin) {
    if (isFirst) {
      SubPolicy.SuppressSpecifiers = false;
      isFirst = false;
    } else {
      if (!isFirst) Out << ", ";
      SubPolicy.SuppressSpecifiers = true;
    }

    (*Begin)->print(Out, SubPolicy, Indentation);
  }
}

void Decl::dump() {
  print(llvm::errs());
}

llvm::raw_ostream& DeclPrinter::Indent() {
  for (unsigned i = 0; i < Indentation; ++i)
    Out << "  ";
  return Out;
}

void DeclPrinter::ProcessDeclGroup(llvm::SmallVectorImpl<Decl*>& Decls) {
  this->Indent();
  Decl::printGroup(Decls.data(), Decls.size(), Out, Policy, Indentation);
  Out << ";\n";
  Decls.clear();

}

//----------------------------------------------------------------------------
// Common C declarations
//----------------------------------------------------------------------------

void DeclPrinter::VisitDeclContext(DeclContext *DC, bool Indent) {
  if (Indent)
    Indentation += Policy.Indentation;

  llvm::SmallVector<Decl*, 2> Decls;
  for (DeclContext::decl_iterator D = DC->decls_begin(), DEnd = DC->decls_end();
       D != DEnd; ++D) {
    if (!Policy.Dump) {
      // Skip over implicit declarations in pretty-printing mode.
      if (D->isImplicit()) continue;
      // FIXME: Ugly hack so we don't pretty-print the builtin declaration
      // of __builtin_va_list.  There should be some other way to check that.
      if (isa<NamedDecl>(*D) && cast<NamedDecl>(*D)->getNameAsString() ==
          "__builtin_va_list")
        continue;
    }

    // The next bits of code handles stuff like "struct {int x;} a,b"; we're
    // forced to merge the declarations because there's no other way to
    // refer to the struct in question.  This limited merging is safe without
    // a bunch of other checks because it only merges declarations directly
    // referring to the tag, not typedefs.
    //
    // Check whether the current declaration should be grouped with a previous
    // unnamed struct.
    QualType CurDeclType = getDeclType(*D);
    if (!Decls.empty() && !CurDeclType.isNull()) {
      QualType BaseType = GetBaseType(CurDeclType);
      if (!BaseType.isNull() && isa<TagType>(BaseType) &&
          cast<TagType>(BaseType)->getDecl() == Decls[0]) {
        Decls.push_back(*D);
        continue;
      }
    }

    // If we have a merged group waiting to be handled, handle it now.
    if (!Decls.empty())
      ProcessDeclGroup(Decls);

    // If the current declaration is an unnamed tag type, save it
    // so we can merge it with the subsequent declaration(s) using it.
    if (isa<TagDecl>(*D) && !cast<TagDecl>(*D)->getIdentifier()) {
      Decls.push_back(*D);
      continue;
    }
    this->Indent();
    Visit(*D);
    
    // FIXME: Need to be able to tell the DeclPrinter when 
    const char *Terminator = 0;
    if (isa<FunctionDecl>(*D) && 
        cast<FunctionDecl>(*D)->isThisDeclarationADefinition())
      Terminator = 0;
    else if (isa<ObjCMethodDecl>(*D) && cast<ObjCMethodDecl>(*D)->getBody())
      Terminator = 0;
    else if (isa<NamespaceDecl>(*D) || isa<LinkageSpecDecl>(*D) ||
             isa<ObjCImplementationDecl>(*D) || 
             isa<ObjCInterfaceDecl>(*D) ||
             isa<ObjCProtocolDecl>(*D) ||
             isa<ObjCCategoryImplDecl>(*D) ||
             isa<ObjCCategoryDecl>(*D))
      Terminator = 0;
    else if (isa<EnumConstantDecl>(*D)) {
      DeclContext::decl_iterator Next = D;
      ++Next;
      if (Next != DEnd)
        Terminator = ",";
    } else
      Terminator = ";";

    if (Terminator)
      Out << Terminator;
    Out << "\n";
  }

  if (!Decls.empty())
    ProcessDeclGroup(Decls);

  if (Indent)
    Indentation -= Policy.Indentation;
}

void DeclPrinter::VisitTranslationUnitDecl(TranslationUnitDecl *D) {
  VisitDeclContext(D, false);
}

void DeclPrinter::VisitTypedefDecl(TypedefDecl *D) {
  std::string S = D->getNameAsString();
  D->getUnderlyingType().getAsStringInternal(S, Policy);
  if (!Policy.SuppressSpecifiers)
    Out << "typedef ";
  Out << S;
}

void DeclPrinter::VisitEnumDecl(EnumDecl *D) {
  Out << "enum " << D->getNameAsString() << " {\n";
  VisitDeclContext(D);
  Indent() << "}";
}

void DeclPrinter::VisitRecordDecl(RecordDecl *D) {
  Out << D->getKindName();
  if (D->getIdentifier()) {
    Out << " ";
    Out << D->getNameAsString();
  }
  
  if (D->isDefinition()) {
    Out << " {\n";
    VisitDeclContext(D);
    Indent() << "}";
  }
}

void DeclPrinter::VisitEnumConstantDecl(EnumConstantDecl *D) {
  Out << D->getNameAsString();
  if (Expr *Init = D->getInitExpr()) {
    Out << " = ";
    Init->printPretty(Out, Context, 0, Policy, Indentation);
  }
}

void DeclPrinter::VisitFunctionDecl(FunctionDecl *D) { 
  if (!Policy.SuppressSpecifiers) {
    switch (D->getStorageClass()) {
    case FunctionDecl::None: break;
    case FunctionDecl::Extern: Out << "extern "; break;
    case FunctionDecl::Static: Out << "static "; break;
    case FunctionDecl::PrivateExtern: Out << "__private_extern__ "; break;
    }

    if (D->isInline())           Out << "inline ";
    if (D->isVirtualAsWritten()) Out << "virtual ";
  }

  PrintingPolicy SubPolicy(Policy);
  SubPolicy.SuppressSpecifiers = false;
  std::string Proto = D->getNameAsString();
  if (isa<FunctionType>(D->getType().getTypePtr())) {
    const FunctionType *AFT = D->getType()->getAsFunctionType();

    const FunctionProtoType *FT = 0;
    if (D->hasWrittenPrototype())
      FT = dyn_cast<FunctionProtoType>(AFT);

    Proto += "(";
    if (FT) {
      llvm::raw_string_ostream POut(Proto);
      DeclPrinter ParamPrinter(POut, Context, SubPolicy, Indentation);
      for (unsigned i = 0, e = D->getNumParams(); i != e; ++i) {
        if (i) POut << ", ";
        ParamPrinter.VisitParmVarDecl(D->getParamDecl(i));
      }
    
      if (FT->isVariadic()) {
        if (D->getNumParams()) POut << ", ";
        POut << "...";
      }
    } else if (D->isThisDeclarationADefinition() && !D->hasPrototype()) {
      for (unsigned i = 0, e = D->getNumParams(); i != e; ++i) {
        if (i)
          Proto += ", ";
        Proto += D->getParamDecl(i)->getNameAsString();
      }
    }

    Proto += ")";
    if (D->hasAttr<NoReturnAttr>())
      Proto += " __attribute((noreturn))";
    if (CXXConstructorDecl *CDecl = dyn_cast<CXXConstructorDecl>(D)) {
      if (CDecl->getNumBaseOrMemberInitializers() > 0) {
        Proto += " : ";
        Out << Proto;
        Proto.clear();
        for (CXXConstructorDecl::init_const_iterator B = CDecl->init_begin(), 
             E = CDecl->init_end();
             B != E; ++B) {
          CXXBaseOrMemberInitializer * BMInitializer = (*B);
          if (B != CDecl->init_begin())
            Out << ", ";
          bool hasArguments = (BMInitializer->arg_begin() != 
                               BMInitializer->arg_end());
          if (BMInitializer->isMemberInitializer()) {
            FieldDecl *FD = BMInitializer->getMember();
            Out <<  FD->getNameAsString();
          }
          else // FIXME. skip dependent types for now.
            if (const RecordType *RT = 
                BMInitializer->getBaseClass()->getAs<RecordType>()) {
              const CXXRecordDecl *BaseDecl = 
                cast<CXXRecordDecl>(RT->getDecl());
              Out << BaseDecl->getNameAsString();
          }
          if (hasArguments) {
            Out << "(";
            for (CXXBaseOrMemberInitializer::const_arg_iterator BE = 
                 BMInitializer->const_arg_begin(), 
                 EE =  BMInitializer->const_arg_end(); BE != EE; ++BE) {
              if (BE != BMInitializer->const_arg_begin())
                Out<< ", ";
              const Expr *Exp = (*BE);
              Exp->printPretty(Out, Context, 0, Policy, Indentation);
            }
            Out << ")";
          } else
            Out << "()";
        }
      }
    }
    else if (CXXDestructorDecl *DDecl = dyn_cast<CXXDestructorDecl>(D)) {
      if (DDecl->getNumBaseOrMemberDestructions() > 0) {
        // List order of base/member destruction for visualization purposes.
        assert (D->isThisDeclarationADefinition() && "Destructor with dtor-list");
        Proto += "/* : ";
        for (CXXDestructorDecl::destr_const_iterator *B = DDecl->destr_begin(), 
             *E = DDecl->destr_end();
             B != E; ++B) {
          uintptr_t BaseOrMember = (*B);
          if (B != DDecl->destr_begin())
            Proto += ", ";

          if (DDecl->isMemberToDestroy(BaseOrMember)) {
            FieldDecl *FD = DDecl->getMemberToDestroy(BaseOrMember);
            Proto += "~";
            Proto += FD->getNameAsString();
          }
          else // FIXME. skip dependent types for now.
            if (const RecordType *RT = 
                  DDecl->getAnyBaseClassToDestroy(BaseOrMember)
                    ->getAs<RecordType>()) {
              const CXXRecordDecl *BaseDecl = 
                cast<CXXRecordDecl>(RT->getDecl());
              Proto += "~";
              Proto += BaseDecl->getNameAsString();
            }
          Proto += "()";
        }
        Proto += " */";
      }
    }
    else
      AFT->getResultType().getAsStringInternal(Proto, Policy);
  } else {
    D->getType().getAsStringInternal(Proto, Policy);
  }

  Out << Proto;

  if (D->isPure())
    Out << " = 0";
  else if (D->isDeleted())
    Out << " = delete";
  else if (D->isThisDeclarationADefinition()) {
    if (!D->hasPrototype() && D->getNumParams()) {
      // This is a K&R function definition, so we need to print the
      // parameters.
      Out << '\n';
      DeclPrinter ParamPrinter(Out, Context, SubPolicy, Indentation);
      Indentation += Policy.Indentation;
      for (unsigned i = 0, e = D->getNumParams(); i != e; ++i) {
        Indent();
        ParamPrinter.VisitParmVarDecl(D->getParamDecl(i));
        Out << ";\n";
      }
      Indentation -= Policy.Indentation;
    } else
      Out << ' ';

    D->getBody()->printPretty(Out, Context, 0, SubPolicy, Indentation);
    Out << '\n';
  }
}

void DeclPrinter::VisitFieldDecl(FieldDecl *D) {
  if (!Policy.SuppressSpecifiers && D->isMutable())
    Out << "mutable ";

  std::string Name = D->getNameAsString();
  D->getType().getAsStringInternal(Name, Policy);
  Out << Name;

  if (D->isBitField()) {
    Out << " : ";
    D->getBitWidth()->printPretty(Out, Context, 0, Policy, Indentation);
  }
}

void DeclPrinter::VisitVarDecl(VarDecl *D) {
  if (!Policy.SuppressSpecifiers && D->getStorageClass() != VarDecl::None)
    Out << VarDecl::getStorageClassSpecifierString(D->getStorageClass()) << " ";

  if (!Policy.SuppressSpecifiers && D->isThreadSpecified())
    Out << "__thread ";

  std::string Name = D->getNameAsString();
  QualType T = D->getType();
  if (OriginalParmVarDecl *Parm = dyn_cast<OriginalParmVarDecl>(D))
    T = Parm->getOriginalType();
  T.getAsStringInternal(Name, Policy);
  Out << Name;
  if (D->getInit()) {
    if (D->hasCXXDirectInitializer())
      Out << "(";
    else
      Out << " = ";
    D->getInit()->printPretty(Out, Context, 0, Policy, Indentation);
    if (D->hasCXXDirectInitializer())
      Out << ")";
  }
}

void DeclPrinter::VisitParmVarDecl(ParmVarDecl *D) {
  VisitVarDecl(D);
}

void DeclPrinter::VisitOriginalParmVarDecl(OriginalParmVarDecl *D) {
  VisitVarDecl(D);
}

void DeclPrinter::VisitFileScopeAsmDecl(FileScopeAsmDecl *D) {
  Out << "__asm (";
  D->getAsmString()->printPretty(Out, Context, 0, Policy, Indentation);
  Out << ")";
}

//----------------------------------------------------------------------------
// C++ declarations
//----------------------------------------------------------------------------
void DeclPrinter::VisitOverloadedFunctionDecl(OverloadedFunctionDecl *D) {
  assert(false && 
         "OverloadedFunctionDecls aren't really decls and are never printed");
}

void DeclPrinter::VisitNamespaceDecl(NamespaceDecl *D) {
  Out << "namespace " << D->getNameAsString() << " {\n";
  VisitDeclContext(D);
  Indent() << "}";
}

void DeclPrinter::VisitUsingDirectiveDecl(UsingDirectiveDecl *D) {
  Out << "using namespace ";
  if (D->getQualifier())
    D->getQualifier()->print(Out, Policy);
  Out << D->getNominatedNamespace()->getNameAsString();
}

void DeclPrinter::VisitNamespaceAliasDecl(NamespaceAliasDecl *D) {
  Out << "namespace " << D->getNameAsString() << " = ";
  if (D->getQualifier())
    D->getQualifier()->print(Out, Policy);
  Out << D->getAliasedNamespace()->getNameAsString();
}

void DeclPrinter::VisitCXXRecordDecl(CXXRecordDecl *D) {
  Out << D->getKindName();
  if (D->getIdentifier()) {
    Out << " ";
    Out << D->getNameAsString();
  }
  
  if (D->isDefinition()) {
    // Print the base classes
    if (D->getNumBases()) {
      Out << " : ";
      for(CXXRecordDecl::base_class_iterator Base = D->bases_begin(),
                                          BaseEnd = D->bases_end();
          Base != BaseEnd; ++Base) {
        if (Base != D->bases_begin())
          Out << ", ";

        if (Base->isVirtual())
          Out << "virtual ";

        switch(Base->getAccessSpecifierAsWritten()) {
        case AS_none:      break;
        case AS_public:    Out << "public "; break;
        case AS_protected: Out << "protected "; break;
        case AS_private:   Out << " private "; break;
        }

        Out << Base->getType().getAsString(Policy);
      }
    }

    // Print the class definition
    // FIXME: Doesn't print access specifiers, e.g., "public:"
    Out << " {\n";
    VisitDeclContext(D);
    Indent() << "}";
  }  
}

void DeclPrinter::VisitLinkageSpecDecl(LinkageSpecDecl *D) {
  const char *l;
  if (D->getLanguage() == LinkageSpecDecl::lang_c)
    l = "C";
  else {
    assert(D->getLanguage() == LinkageSpecDecl::lang_cxx &&
           "unknown language in linkage specification");
    l = "C++";
  }

  Out << "extern \"" << l << "\" ";
  if (D->hasBraces()) {
    Out << "{\n";
    VisitDeclContext(D);
    Indent() << "}";
  } else
    Visit(*D->decls_begin());
}

void DeclPrinter::VisitTemplateDecl(TemplateDecl *D) {
  Out << "template <";
  
  TemplateParameterList *Params = D->getTemplateParameters();
  for (unsigned i = 0, e = Params->size(); i != e; ++i) {
    if (i != 0)
      Out << ", ";
    
    const Decl *Param = Params->getParam(i);
    if (const TemplateTypeParmDecl *TTP = 
          dyn_cast<TemplateTypeParmDecl>(Param)) {
      
      QualType ParamType = 
        Context.getTypeDeclType(const_cast<TemplateTypeParmDecl*>(TTP));

      if (TTP->wasDeclaredWithTypename())
        Out << "typename ";
      else
        Out << "class ";

      if (TTP->isParameterPack())
        Out << "... ";
      
      Out << ParamType.getAsString(Policy);

      if (TTP->hasDefaultArgument()) {
        Out << " = ";
        Out << TTP->getDefaultArgument().getAsString(Policy);
      };
    } else if (const NonTypeTemplateParmDecl *NTTP = 
                 dyn_cast<NonTypeTemplateParmDecl>(Param)) {
      Out << NTTP->getType().getAsString(Policy);

      if (IdentifierInfo *Name = NTTP->getIdentifier()) {
        Out << ' ';
        Out << Name->getName();
      }
      
      if (NTTP->hasDefaultArgument()) {
        Out << " = ";
        NTTP->getDefaultArgument()->printPretty(Out, Context, 0, Policy, 
                                                Indentation);
      }
    }
  }
  
  Out << "> ";

  Visit(D->getTemplatedDecl());
}

//----------------------------------------------------------------------------
// Objective-C declarations
//----------------------------------------------------------------------------

void DeclPrinter::VisitObjCClassDecl(ObjCClassDecl *D) {
  Out << "@class ";
  for (ObjCClassDecl::iterator I = D->begin(), E = D->end();
       I != E; ++I) {
    if (I != D->begin()) Out << ", ";
    Out << (*I)->getNameAsString();
  }
}

void DeclPrinter::VisitObjCMethodDecl(ObjCMethodDecl *OMD) {
  if (OMD->isInstanceMethod())
    Out << "- ";
  else 
    Out << "+ ";
  if (!OMD->getResultType().isNull())
    Out << '(' << OMD->getResultType().getAsString(Policy) << ")";
  
  std::string name = OMD->getSelector().getAsString();
  std::string::size_type pos, lastPos = 0;
  for (ObjCMethodDecl::param_iterator PI = OMD->param_begin(),
       E = OMD->param_end(); PI != E; ++PI) {
    // FIXME: selector is missing here!    
    pos = name.find_first_of(":", lastPos);
    Out << " " << name.substr(lastPos, pos - lastPos);
    Out << ":(" << (*PI)->getType().getAsString(Policy) << ")"
        << (*PI)->getNameAsString(); 
    lastPos = pos + 1;
  }
    
  if (OMD->param_begin() == OMD->param_end())
    Out << " " << name;
    
  if (OMD->isVariadic())
      Out << ", ...";
  
  if (OMD->getBody()) {
    Out << ' ';
    OMD->getBody()->printPretty(Out, Context, 0, Policy);
    Out << '\n';
  }
}

void DeclPrinter::VisitObjCImplementationDecl(ObjCImplementationDecl *OID) {
  std::string I = OID->getNameAsString();
  ObjCInterfaceDecl *SID = OID->getSuperClass();

  if (SID)
    Out << "@implementation " << I << " : " << SID->getNameAsString();
  else
    Out << "@implementation " << I;
  Out << "\n";
  VisitDeclContext(OID, false);
  Out << "@end";
}

void DeclPrinter::VisitObjCInterfaceDecl(ObjCInterfaceDecl *OID) {
  std::string I = OID->getNameAsString();
  ObjCInterfaceDecl *SID = OID->getSuperClass();

  if (SID)
    Out << "@interface " << I << " : " << SID->getNameAsString();
  else
    Out << "@interface " << I;
  
  // Protocols?
  const ObjCList<ObjCProtocolDecl> &Protocols = OID->getReferencedProtocols();
  if (!Protocols.empty()) {
    for (ObjCList<ObjCProtocolDecl>::iterator I = Protocols.begin(),
         E = Protocols.end(); I != E; ++I)
      Out << (I == Protocols.begin() ? '<' : ',') << (*I)->getNameAsString();
  }
  
  if (!Protocols.empty())
    Out << "> ";
  
  if (OID->ivar_size() > 0) {
    Out << "{\n";
    Indentation += Policy.Indentation;
    for (ObjCInterfaceDecl::ivar_iterator I = OID->ivar_begin(),
         E = OID->ivar_end(); I != E; ++I) {
      Indent() << (*I)->getType().getAsString(Policy)
          << ' '  << (*I)->getNameAsString() << ";\n";      
    }
    Indentation -= Policy.Indentation;
    Out << "}\n";
  }
  
  VisitDeclContext(OID, false);
  Out << "@end";
  // FIXME: implement the rest...
}

void DeclPrinter::VisitObjCForwardProtocolDecl(ObjCForwardProtocolDecl *D) {
  Out << "@protocol ";
  for (ObjCForwardProtocolDecl::protocol_iterator I = D->protocol_begin(), 
         E = D->protocol_end();
       I != E; ++I) {
    if (I != D->protocol_begin()) Out << ", ";
    Out << (*I)->getNameAsString();
  }
}

void DeclPrinter::VisitObjCProtocolDecl(ObjCProtocolDecl *PID) {
  Out << "@protocol " << PID->getNameAsString() << '\n';
  VisitDeclContext(PID, false);
  Out << "@end";
}

void DeclPrinter::VisitObjCCategoryImplDecl(ObjCCategoryImplDecl *PID) {
  Out << "@implementation "
      << PID->getClassInterface()->getNameAsString()
      << '(' << PID->getNameAsString() << ")\n";  

  VisitDeclContext(PID, false);
  Out << "@end";
  // FIXME: implement the rest...
}

void DeclPrinter::VisitObjCCategoryDecl(ObjCCategoryDecl *PID) {
  Out << "@interface " 
      << PID->getClassInterface()->getNameAsString()
      << '(' << PID->getNameAsString() << ")\n";
  VisitDeclContext(PID, false);
  Out << "@end";
  
  // FIXME: implement the rest...
}

void DeclPrinter::VisitObjCCompatibleAliasDecl(ObjCCompatibleAliasDecl *AID) {
  Out << "@compatibility_alias " << AID->getNameAsString() 
      << ' ' << AID->getClassInterface()->getNameAsString() << ";\n";  
}

/// PrintObjCPropertyDecl - print a property declaration.
///
void DeclPrinter::VisitObjCPropertyDecl(ObjCPropertyDecl *PDecl) {
  if (PDecl->getPropertyImplementation() == ObjCPropertyDecl::Required)
    Out << "@required\n";
  else if (PDecl->getPropertyImplementation() == ObjCPropertyDecl::Optional)
    Out << "@optional\n";
  
  Out << "@property";
  if (PDecl->getPropertyAttributes() != ObjCPropertyDecl::OBJC_PR_noattr) {
    bool first = true;
    Out << " (";
    if (PDecl->getPropertyAttributes() & 
        ObjCPropertyDecl::OBJC_PR_readonly) {
      Out << (first ? ' ' : ',') << "readonly";
      first = false;
  }
      
  if (PDecl->getPropertyAttributes() & ObjCPropertyDecl::OBJC_PR_getter) {
    Out << (first ? ' ' : ',') << "getter = "
        << PDecl->getGetterName().getAsString();
    first = false;
  }
  if (PDecl->getPropertyAttributes() & ObjCPropertyDecl::OBJC_PR_setter) {
    Out << (first ? ' ' : ',') << "setter = "
        << PDecl->getSetterName().getAsString();
    first = false;
  }
      
  if (PDecl->getPropertyAttributes() & ObjCPropertyDecl::OBJC_PR_assign) {
    Out << (first ? ' ' : ',') << "assign";
    first = false;
  }
      
  if (PDecl->getPropertyAttributes() &
      ObjCPropertyDecl::OBJC_PR_readwrite) {
    Out << (first ? ' ' : ',') << "readwrite";
    first = false;
  }
      
  if (PDecl->getPropertyAttributes() & ObjCPropertyDecl::OBJC_PR_retain) {
    Out << (first ? ' ' : ',') << "retain";
    first = false;
  }
      
  if (PDecl->getPropertyAttributes() & ObjCPropertyDecl::OBJC_PR_copy) {
    Out << (first ? ' ' : ',') << "copy";
    first = false;
  }
      
  if (PDecl->getPropertyAttributes() & 
      ObjCPropertyDecl::OBJC_PR_nonatomic) {
    Out << (first ? ' ' : ',') << "nonatomic";
    first = false;
  }
  Out << " )";
  }
  Out << ' ' << PDecl->getType().getAsString(Policy)
  << ' ' << PDecl->getNameAsString();
}

void DeclPrinter::VisitObjCPropertyImplDecl(ObjCPropertyImplDecl *PID) {
  if (PID->getPropertyImplementation() == ObjCPropertyImplDecl::Synthesize)
    Out << "@synthesize ";
  else
    Out << "@dynamic ";
  Out << PID->getPropertyDecl()->getNameAsString();
  if (PID->getPropertyIvarDecl())
    Out << "=" << PID->getPropertyIvarDecl()->getNameAsString();
}