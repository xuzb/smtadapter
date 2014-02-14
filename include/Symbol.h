#ifndef LASER_SYMBOL_H    // -*- C++ -*-
#define LASER_SYMBOL_H
#include <string>
#include <sstream>
#include <stdint.h>

namespace solver {

enum BinOpcode {
  BO_Mul, BO_Div, BO_Rem,
  BO_Add, BO_Sub,
  BO_Shl, BO_Shr,
  BO_LT, BO_GT, BO_LE, BO_GE,   // [C99 6.5.8] Relational operators.
  BO_EQ, BO_NE,
  BO_And,                       // [C99 6.5.10] Bitwise AND operator.
  BO_Xor,                       // [C99 6.5.11] Bitwise XOR operator.
  BO_Or,                        // [C99 6.5.12] Bitwise OR operator.
  BO_LAnd,                      // [C99 6.5.13] Logical AND operator.
  BO_LOr                        // [C99 6.5.14] Logical OR operator.
};

enum UnaryOpcode {
  UO_Minus,      // [C99 6.5.3.3] Unary arithmetic
  UO_Not, UO_LNot,        // [C99 6.5.3.3] Unary arithmetic
};

class SymExpr {
public:
  virtual unsigned getTypeBitSize() const = 0;
  virtual bool isSigned() const = 0;
  virtual bool isSymbol() const { return false; }
  virtual bool isBinSymExpr() const { return false; }
  virtual bool isUnarySymExpr() const { return false; }
  virtual bool isCastSymExpr() const { return false; }
  virtual bool isConstExpr() const { return false; }
  virtual bool isElemSymExpr() const {return false;}
};

class Symbol : public SymExpr {
protected:
  unsigned ID;
  
public:
  Symbol(unsigned id) : ID(id) {}
  virtual bool isSigned() const {return false;} 
  virtual unsigned getSymbolID() const {return ID;}
  virtual bool isSymbol() const { return true; }
  std::string getSymName() const {
    std::string os;
    std::stringstream ss(os);
    ss << "$" << ID;
    return ss.str();
  }

  static bool classof(const SymExpr *se) {
    return se->isSymbol();
  }
};

class ElemSymExpr : public SymExpr {
protected:
  const SymExpr *baseExpr;
  const SymExpr *indexExpr;

public:
  ElemSymExpr(SymExpr *base, SymExpr *index) :
		baseExpr(base), indexExpr(index) {}

  const SymExpr *getBaseExpr() const { return baseExpr; }
  const SymExpr *getIndexExpr() const { return indexExpr; }
  
  virtual bool isSigned() const {return false;}
  virtual bool isElemSymExpr() const {return true;}
  static bool classof(const SymExpr *se) {
	return se->isElemSymExpr();
  }
};

class BinSymExpr : public SymExpr {
protected:
  const SymExpr *lhs;
  const SymExpr *rhs;
  BinOpcode op;

public:
  BinSymExpr(const SymExpr *l, const SymExpr *r, BinOpcode o)
    : lhs(l), rhs(r), op(o) {}
  
  const SymExpr *getLHS() const { return lhs; }
  const SymExpr *getRHS() const { return rhs; }
  BinOpcode getOpcode() const { return op; }

  virtual bool isBinSymExpr() const { return true; }

  static bool classof(const SymExpr *se) {
    return se->isBinSymExpr();
  }
};

class UnarySymExpr : public SymExpr {
protected:
  const SymExpr *operand;
  UnaryOpcode uo;

public:
  UnarySymExpr(const SymExpr *se, UnaryOpcode o)
    : operand(se), uo(o) {}

  const SymExpr *getOperand() const { return operand; }
  UnaryOpcode getUnaryOpcode() const { return uo; }
  
  virtual bool isUnarySymExpr() const { return true; }

  static bool classof(const SymExpr *se) {
    return se->isUnarySymExpr();
  }
};

class CastSymExpr : public SymExpr {
  const SymExpr *operand;

public:
  CastSymExpr(const SymExpr *op) : operand(op) {}
  
  const SymExpr *getOperand() const { return operand; }
  virtual bool isCastSymExpr() const { return true; }

  static bool classof(const SymExpr *se) {
    return se->isCastSymExpr();
  }
};

class ConstExpr : public SymExpr {
protected:
  int64_t val;
  unsigned bitwidth;
  bool bSigned;

public:
  ConstExpr(int64_t v, unsigned width, bool s)
    : val(v), bitwidth(width), bSigned(s) {}

  int64_t getValue() const { return val; }
  
  virtual bool isSigned() const {
    return bSigned;
  }
  
  virtual unsigned getTypeBitSize() const {
    return bitwidth;
  }

  virtual bool isConstExpr() const { return true; }

  static bool classof(const SymExpr *se) {
    return se->isConstExpr();
  }
};

}

#endif
