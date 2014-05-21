#ifndef SMTADAPTER_SOLVER_CONTEXT_H     // -*- C++ -*-
#define SMTADAPTER_SOLVER_CONTEXT_H

#include <assert.h>

namespace llvm {
class Type;
}

namespace smt {

using llvm::Type;

// An base class to compute symbol size.
class SolverContext {
public:
  virtual unsigned getArrayIndexTypeSize() {
    return 8;
  }

  unsigned getArrayIndexTypeSizeInBits() {
    return getArrayIndexTypeSize() * 8;
  }

  // Get type size in bytes.
  virtual unsigned getTypeSize(Type *type) {
    assert(0 && "Cannot call getTypeSize in SolverContext.");
  }

  unsigned getTypeSizeInBits(Type *type) {
    return getTypeSize(type) * 8;
  }
};

  
}

#endif
