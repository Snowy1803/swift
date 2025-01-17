//===--- Outlining.h - Value operation outlining ----------------*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// This file defines interfaces for outlining value witnesses.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IRGEN_OUTLINING_H
#define SWIFT_IRGEN_OUTLINING_H

#include "IRGen.h"
#include "LocalTypeDataKind.h"
#include "swift/Basic/LLVM.h"
#include "llvm/ADT/MapVector.h"

namespace llvm {
  class Value;
  class Type;
}

namespace swift {
class CanGenericSignature;
class CanType;
enum IsInitialization_t : bool;
enum IsTake_t : bool;
class SILType;

namespace irgen {
class Address;
class Explosion;
class IRGenFunction;
class IRGenModule;
class TypeInfo;

enum LayoutIsNeeded_t : bool {
  LayoutIsNotNeeded = false,
  LayoutIsNeeded = true
};

enum DeinitIsNeeded_t : bool {
  DeinitIsNotNeeded = false,
  DeinitIsNeeded = true
};

/// A helper class for emitting outlined value operations.
///
/// The use-pattern for this class is:
///   - construct it
///   - collect all the metadata that will be required in order to perform
///     the value operations
///   - emit the call to the outlined copy/destroy helper
class OutliningMetadataCollector {
public:
  IRGenFunction &IGF;
  const unsigned needsLayout : 1;
  const unsigned needsDeinit : 1;

private:
  llvm::MapVector<LocalTypeDataKey, llvm::Value *> Values;
  friend class IRGenModule;

public:
  OutliningMetadataCollector(IRGenFunction &IGF, LayoutIsNeeded_t needsLayout,
                             DeinitIsNeeded_t needsDeinitTypes)
      : IGF(IGF), needsLayout(needsLayout), needsDeinit(needsDeinitTypes) {}

  void collectTypeMetadataForLayout(SILType type);

  void emitCallToOutlinedCopy(Address dest, Address src,
                              SILType T, const TypeInfo &ti,
                              IsInitialization_t isInit, IsTake_t isTake) const;
  void emitCallToOutlinedDestroy(Address addr, SILType T,
                                 const TypeInfo &ti) const;
  void emitCallToOutlinedRelease(Address addr, SILType T, const TypeInfo &ti,
                                 Atomicity atomicity) const;

private:
  void collectFormalTypeMetadata(CanType type);
  void collectRepresentationTypeMetadata(SILType ty);

  void addMetadataArguments(SmallVectorImpl<llvm::Value *> &args) const ;
  void addMetadataParameterTypes(SmallVectorImpl<llvm::Type *> &paramTys) const;
  void bindMetadataParameters(IRGenFunction &helperIGF,
                              Explosion &params) const;
};

std::pair<CanType, CanGenericSignature>
getTypeAndGenericSignatureForManglingOutlineFunction(SILType type);


}
}

#endif
