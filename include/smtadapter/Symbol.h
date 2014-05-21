#ifndef SMTADAPTER_SYMBOL_H    // -*- C++ -*-
#define SMTADAPTER_SYMBOL_H
#include "SolverContext.h"
#include <string>
#include <sstream>
#include <vector>
#include <assert.h>

namespace llvm {
class Type;
class raw_ostream;
}

namespace smt {

using llvm::Type;
using llvm::raw_ostream;

enum ArithOpcode {
  BO_Mul, BO_SDiv, BO_UDiv, BO_SRem, BO_URem,
  BO_Add, BO_Sub,
  BO_Shl, BO_Shr,
  BO_And,                       // [C99 6.5.10] Bitwise AND operator.
  BO_Xor,                       // [C99 6.5.11] Bitwise XOR operator.
  BO_Or                        // [C99 6.5.12] Bitwise OR operator.
};

enum LogicalOpcode {
  BO_SLT, BO_ULT, BO_SGT, BO_UGT,
  BO_SLE, BO_ULE, BO_SGE, BO_UGE,
  BO_EQ, BO_NE,
  BO_LAnd, BO_LOr // [C99 6.5.13/14] Logical AND/OR operator.
};

enum UnaryOpcode {
  UO_Minus,      // [C99 6.5.3.3] Unary arithmetic
  UO_Not, UO_LNot        // [C99 6.5.3.3] Unary arithmetic
};

class SymExpr {
public:
    enum Kind {
      BEGIN_SYMEXPR,
      S_ElemSymExpr = BEGIN_SYMEXPR,

      BEGIN_BINSYMEXPR,
      S_ArithSymExpr = BEGIN_BINSYMEXPR,
      S_LogicalSymExpr,
      END_BINSYMEXPR = S_LogicalSymExpr,

      BEGIN_CASTSYMEXPR,
      S_TruncSymExpr = BEGIN_CASTSYMEXPR,
      S_ExtendSymExpr,
      END_CASTSYMEXPR = S_ExtendSymExpr,
      
      S_UnarySymExpr,
      S_ConstExpr,
      
      BEGIN_SYMBOL,
      S_ScalarSymbol = BEGIN_SYMBOL,
      S_RegionSymbol,
      END_SYMBOL = S_RegionSymbol,
      
      END_SYMEXPR = S_RegionSymbol
  };

protected:
  Kind kind;
  
public:
  SymExpr(Kind k) : kind(k) {}
  Kind getKind() const { return kind; }
  virtual std::string toString() const {
    // FIXME: Implemented toString function
    assert(0 && "The base toString() function is not impleneted.");
  }

  virtual void print(raw_ostream &os) const {
    assert(0 && "Cannot implement print() on SymExpr.");
  }

  // For llvm compatibility. If you don't need llvm, overide this function.
  virtual unsigned getTypeSizeInBits(SolverContext &ctx) const {
    return ctx.getTypeSizeInBits(getType());
  }
  
  virtual Type *getType() const {
    assert(0 && "Cannot call getType on SymExpr.");
  }

};

class Symbol : public SymExpr {
protected:
  unsigned ID;
  
public:
  Symbol(Kind k, unsigned id) : SymExpr(k), ID(id) {}
  virtual unsigned getSymbolID() const {return ID;}
  std::string getSymName() const {
    std::string os;
    std::stringstream ss(os);
    ss << "$" << ID;
    return ss.str();
  }

  static bool classof(const SymExpr *se) {
    return se->getKind() >= BEGIN_SYMBOL && se->getKind() <= END_SYMBOL;
  }
};

class ScalarSymbol : public Symbol {
public:
  ScalarSymbol(unsigned id)
    : Symbol(S_ScalarSymbol, id) {}

  static bool classof(const SymExpr *se) {
    return se->getKind() == S_ScalarSymbol;
  }
};

class RegionSymbol : public Symbol {
public:
  RegionSymbol(unsigned id)
    : Symbol(S_RegionSymbol, id) {}

  virtual unsigned getTypeSizeInBits(SolverContext &) const {
    assert(0 && "Should not call getTypeSizeInBits() on RegionSymbol.");
  }

  virtual unsigned getNumberDimension(SolverContext &) const = 0;

  // For llvm compatibility. If you don't need llvm, overide this function.
  virtual unsigned getElementTypeSizeInBits(SolverContext &ctx) const {
    return ctx.getTypeSizeInBits(getElementType());
  }
  
  virtual Type *getElementType() const {
    assert(0 && "Cannot call getElementType in RegionSymbol.");
  }
  
  static bool classof(const SymExpr *se) {
    return se->getKind() == S_RegionSymbol;
  }
};

class ElemSymExpr : public SymExpr {
protected:
  const SymExpr *baseExpr;
  const SymExpr *indexExpr;

public:
  ElemSymExpr(SymExpr *base, SymExpr *index) :
		SymExpr(S_ElemSymExpr), baseExpr(base), indexExpr(index) {}

  const SymExpr *getBaseExpr() const { return baseExpr; }
  const SymExpr *getIndexExpr() const { return indexExpr; }

  virtual unsigned getTypeSizeInBits(SolverContext &) const {
    assert(0 && "Should not call getTypeSizeInBits() on ElemSymExpr.");
  }
  
  static bool classof(const SymExpr *se) {
    return se->getKind() == S_ElemSymExpr;
  }
};

class BinSymExpr : public SymExpr {
protected:
  const SymExpr *lhs;
  const SymExpr *rhs;

public:
  BinSymExpr(const SymExpr *l, const SymExpr *r, Kind k)
    : SymExpr(k), lhs(l), rhs(r) {}
  
  const SymExpr *getLHS() const { return lhs; }
  const SymExpr *getRHS() const { return rhs; }

  static bool classof(const SymExpr *se) {
    return se->getKind() >= BEGIN_BINSYMEXPR && se->getKind() <= END_BINSYMEXPR;
  }
};

class ArithSymExpr : public BinSymExpr {
protected:
  ArithOpcode opcode;

public:
  ArithSymExpr(const SymExpr *l, const SymExpr *r, ArithOpcode op)
    : BinSymExpr(l, r, S_ArithSymExpr), opcode(op) {}

  ArithOpcode getOpcode() const { return opcode; }

  virtual unsigned getTypeSizeInBits(SolverContext &ctx) const {
    return lhs->getTypeSizeInBits(ctx);
  }

  static bool classof(const SymExpr *se) {
    return se->getKind() == S_ArithSymExpr;
  }
};

class LogicalSymExpr : public BinSymExpr {
protected:
  LogicalOpcode opcode;

public:
  LogicalSymExpr(const SymExpr *l, const SymExpr *r, LogicalOpcode op)
    : BinSymExpr(l, r, S_LogicalSymExpr), opcode(op) {}

  LogicalOpcode getOpcode() const { return opcode; }

  virtual unsigned getTypeSizeInBits(SolverContext &) const { return 1; }

  static bool classof(const SymExpr *se) {
    return se->getKind() == S_LogicalSymExpr;
  }
};

class UnarySymExpr : public SymExpr {
protected:
  const SymExpr *operand;
  UnaryOpcode uo;

public:
  UnarySymExpr(const SymExpr *se, UnaryOpcode o)
    : SymExpr(S_UnarySymExpr), operand(se), uo(o) {}

  const SymExpr *getOperand() const { return operand; }
  UnaryOpcode getUnaryOpcode() const { return uo; }

  virtual unsigned getTypeSizeInBits(SolverContext &ctx) const {
    return operand->getTypeSizeInBits(ctx);
  }
  
  static bool classof(const SymExpr *se) {
    return se->getKind() == S_UnarySymExpr;
  }
};

class CastSymExpr : public SymExpr {
protected:
  const SymExpr *operand;

public:
  CastSymExpr(Kind k, const SymExpr *op)
  : SymExpr(k), operand(op) {}
  
  const SymExpr *getOperand() const { return operand; }

  static bool classof(const SymExpr *se) {
    return se->getKind() >= BEGIN_CASTSYMEXPR && se->getKind() <= END_CASTSYMEXPR;
  }
};

class TruncSymExpr : public CastSymExpr {
public:
  TruncSymExpr(const SymExpr *op) : CastSymExpr(S_TruncSymExpr, op) {}

  static bool classof(const SymExpr *se) {
    return se->getKind() == S_TruncSymExpr;
  }
};

class ExtendSymExpr : public CastSymExpr {
protected:
  bool bSignedExt;

public:
  ExtendSymExpr(const SymExpr *op, bool sext)
    : CastSymExpr(S_ExtendSymExpr, op), bSignedExt(sext) {}

  bool isSignedExt() const { return bSignedExt; }
  
  static bool classof(const SymExpr *se) {
    return se->getKind() == S_ExtendSymExpr;
  }
};

class ConstExpr : public SymExpr {
public:
  ConstExpr() : SymExpr(S_ConstExpr) {}

  virtual long long getValue() const = 0;
  virtual bool isSigned() const = 0;
  
  static bool classof(const SymExpr *se) {
    return se->getKind() == S_ConstExpr;
  }
};

}

#endif
