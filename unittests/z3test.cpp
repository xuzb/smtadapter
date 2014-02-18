#include "../Z3Adapter.h"
#include <sstream>
#include "llvm/ADT/APSInt.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
using namespace solver;

class Z3Symbol : public ScalarSymbol {
public:
  Z3Symbol(unsigned id, unsigned size, bool b) :
    ScalarSymbol(id, size), sign(b) {}

  virtual bool isSigned() const {
    return sign;
  }

private:
  bool sign;
};

class Z3ElemSymExpr : public ElemSymExpr {
public:
  Z3ElemSymExpr(SymExpr *sup, SymExpr *index, bool b) : ElemSymExpr(sup, index), sign(b) {
  }

  virtual bool isSigned() const {
    return sign;
  }

private:
  bool sign;
};

class Z3ArithSymExpr : public ArithSymExpr {
public:
  Z3ArithSymExpr(const SymExpr *l, const SymExpr *r, ArithOpcode o)
    : ArithSymExpr(l, r, o) {}
};

class Z3LogicalSymExpr : public LogicalSymExpr {
public:
  Z3LogicalSymExpr(const SymExpr *l, const SymExpr *r, LogicalOpcode o)
    : LogicalSymExpr(l, r, o){}
};

class Z3UnarySymExpr : public UnarySymExpr {
public:
  Z3UnarySymExpr(const SymExpr *se, UnaryOpcode o) 
    : UnarySymExpr(se, o) {}

};

class Z3TruncSymExpr : public TruncSymExpr {
  unsigned bitsize;

public:
  Z3TruncSymExpr(int bs, const SymExpr *op) : TruncSymExpr(op), bitsize(bs) {}

  virtual unsigned getTypeBitSize() const { return bitsize; }
};

class Z3ExtendSymExpr : public ExtendSymExpr {
public:
  Z3ExtendSymExpr(int nbz, bool sext, const SymExpr *op)
    : ExtendSymExpr(op, sext), newBitSize(nbz) {}

  virtual unsigned getTypeBitSize() const {return newBitSize;}
  int getOldBitSize() const {return getOperand()->getTypeBitSize();}

private:
  unsigned newBitSize;
	
};

class Z3ConstExpr : public ConstExpr {
  const APSInt *val;
public:
  Z3ConstExpr(const APSInt *v) : 
	ConstExpr(), val(v) {}

  virtual bool isSigned() const {
    return !val->isUnsigned();
  }

  virtual long long getValue() const { return val->getZExtValue(); }

  virtual unsigned getTypeBitSize() const { return val->getBitWidth(); }
};

void testZ3Symbol();
void testZ3ElemSymExpr();
void testZ3ArithSymExpr();
void testZ3UnarySymExpr();
void testZ3IntCastSymExpr();
void testZ3Adapter();
void testMemLeak();

int main() {
  // Test Z3Symbol
  testZ3Symbol();

  // Test Z3ElemSymExpr
  testZ3ElemSymExpr();

  // Test Z3ArithSymExpr
  testZ3ArithSymExpr();

  //Test Z3UnarySymExpr
  testZ3UnarySymExpr();

  //Test Z3IntCastSymExpr
  testZ3IntCastSymExpr();

  // Test Z3Adapter
  testZ3Adapter();

  // Test Memory Leak
  // testMemLeak();
}

void testZ3Symbol() {
  llvm::errs() << "Test Z3Symbol . . .\n";

  unsigned timeout = 1000;  // 1 second
  Z3Adapter adapter(timeout);

  // Unsigned int x1
  Z3Symbol x1(1, 32, false);

  // Signed short x2
  Z3Symbol x2(2, 16, true);

  z3::expr ex1 = adapter.genZ3Expr(&x1);
  z3::expr ex2 = adapter.genZ3Expr(&x2);

  std::ostringstream oss;
  oss << ex1 << "\n";
  llvm::errs() << oss.str();

  oss.str("");
  oss << ex2 << "\n";
  llvm::errs() << oss.str() << "\n";
}  

void testZ3ElemSymExpr() {
  llvm::errs() << "Test Z3ElemSymbol ...\n";

  unsigned timeout = 1000; // 1 second
  Z3Adapter adapter(timeout);

  // 1 dimension array, int id[index]
  ArraySymbol id(1, 32, 1);
  Z3Symbol index(2, SymExpr::getArrayIndexTypeBitSize(), false);
  Z3ElemSymExpr a(&id, &index, false);
  z3::expr exa = adapter.genZ3Expr(&a);
  std::ostringstream oss;
  oss << exa << "\n";
  llvm::errs() << oss.str();

  // 2 dimension array, int id2[index1][index2]
  ArraySymbol id2(3, 32, 2);
  Z3Symbol index1(4, SymExpr::getArrayIndexTypeBitSize(), true);
  Z3Symbol index2(5, SymExpr::getArrayIndexTypeBitSize(), true);
  Z3ElemSymExpr b(&id2, &index1, true);
  Z3ElemSymExpr c(&b, &index2, true);
  z3::expr exb = adapter.genZ3Expr(&c);
  oss.str("");
  oss << exb << "\n";
  llvm::errs() << oss.str() << "\n";
}

void testZ3ArithSymExpr() {
  llvm::errs() << "Test Z3BinSymbol ...\n";
  unsigned timeout = 1000; // 1 second
  Z3Adapter adapter(timeout);

  // Unsigned int x1 
  Z3Symbol x1(1, 32, false);
  // Unsigned int x2 
  Z3Symbol x2(2, 32, false);
  // Unsigned int x3
  Z3Symbol x3(3, 32, false);

  // (x1 * x2 <= x1 + x3) && (x1 << x2 > x3)
  Z3ArithSymExpr bin1(&x1, &x2, BO_Mul);
  Z3ArithSymExpr bin2(&x1, &x3, BO_Add);
  Z3LogicalSymExpr bin3(&bin1, &bin2, BO_ULE);
  Z3ArithSymExpr bin4(&x1, &x2, BO_Shl);
  Z3LogicalSymExpr bin5(&bin4, &x3, BO_UGT);
  Z3LogicalSymExpr bin(&bin3, &bin5, BO_LAnd);

  z3::expr ex = adapter.genZ3Expr(&bin);
  std::ostringstream oss;
  oss << ex << "\n";
  llvm::errs() << oss.str() << "\n";
}

void testZ3UnarySymExpr() {
  llvm::errs() << "Test Z3UnarySymExpr . . .\n";
  unsigned timeout;
  Z3Adapter adapter(timeout);

  Z3Symbol x1(1, 32, false);
  Z3Symbol x2(2, 32, false);

  // !(~(-x1) > x2)
  Z3UnarySymExpr un1(&x1, UO_Minus);
  Z3UnarySymExpr un2(&un1, UO_Not);
  Z3LogicalSymExpr bin1(&un2, &x2, BO_UGT);
  Z3UnarySymExpr un(&bin1, UO_LNot);

  z3::expr ex = adapter.genZ3Expr(&un);
  std::ostringstream oss;
  oss << ex << "\n";
  llvm::errs() << oss.str() << "\n";
}

void testZ3IntCastSymExpr() {
  llvm::errs() << "Test Z3IntCastSymExpr. . .\n";
  unsigned timeout;
  Z3Adapter adapter(timeout);

  Z3Symbol x1(1, 32, false);
  Z3TruncSymExpr cse1(16, &x1);
  z3::expr ex1 = adapter.genZ3Expr(&cse1);
  std::ostringstream oss;
  oss << ex1 << "\n";
  llvm::errs() << oss.str();

  // Extract
  llvm::APInt v1(64, 1024);
  llvm::APSInt v2(v1, false);
  Z3ConstExpr ce1(&v2);
  Z3TruncSymExpr cse2(8, &ce1);
  z3::expr ex2 = adapter.genZ3Expr(&cse2);
  oss.str("");
  oss << ex2 << "\n";
  llvm::errs() << oss.str();

  // Signed extend
  llvm::APInt v3(32, -1024, true);
  llvm::APSInt v4(v3, true);
  Z3ConstExpr ce2(&v4);
  Z3ExtendSymExpr cse3(64, true, &ce2);
  z3::expr ex3 = adapter.genZ3Expr(&cse3);
  oss.str("");
  oss << ex3 << "\n";
  llvm::errs() << oss.str();

  // Unsigned extend
  llvm::APInt v5(32, 1024, false);
  llvm::APSInt v6(v5, false);
  Z3ConstExpr ce3(&v6);
  Z3ExtendSymExpr cse4(64, false, &ce3);
  z3::expr ex4 = adapter.genZ3Expr(&cse4);
  oss.str("");
  oss << ex4 << "\n";
  llvm::errs() << oss.str() << "\n";
}

void testZ3Adapter() {
  llvm::errs() << "Test Z3Adapter. . .\n";
  unsigned timeout = 1000; // 1 second
  SolverAdapter *adapter = CreateZ3SolverAdapter();

  // x1 * 5 < x2 + 6
  llvm::errs() << "// x1 * 5 < x2 + 6\n";
  Z3Symbol x1(1, 32, false);
  Z3Symbol x2(2, 32, false);
  llvm::APInt v1(32, 5), v2(32, 6);
  llvm::APSInt v3(v1, false), v4(v2, false);
  Z3ConstExpr ce1(&v3), ce2(&v4);
  Z3ArithSymExpr bin1(&x1, &ce1, BO_Mul), bin2(&x2, &ce2, BO_Add);
  Z3LogicalSymExpr bin3(&bin1, &bin2, BO_ULT);
  SymConstraint sc1(&bin3, true);
  adapter->assertSymConstraint(sc1);
  if(adapter->checkSat() == SAT_Satisfiable) {
    adapter->printModel();
  }
  adapter->reset();
  
  // x3 = 0xFFFFFFFF, extract(x3, 16) = x4
  // x5 = unsignedextend(x3, 48 - 32)
  // x6 = signedextend(x4, 32 - 16)
  llvm::errs() << "// x3 = 0xFFFFFFFF, extract(x3, 16) = x4\n";
  llvm::errs() << "// x5 = unsingedextend(x3, 48 - 32)\n";
  llvm::errs() << "// x6 = singedextend(x4, 32 - 16\n";
  Z3Symbol x3(3, 32, false);
  Z3Symbol x4(4, 16, true);
  Z3Symbol x5(5, 48, false);
  Z3Symbol x6(6, 32, true);
  llvm::APInt v5(32, 0xFFFFFFFF);
  llvm::APSInt v6(v5, true);
  Z3ConstExpr ce3(&v6);
  Z3LogicalSymExpr bin4(&x3, &ce3, BO_EQ);
  Z3TruncSymExpr cse1(16, &x3);
  Z3ExtendSymExpr cse2(48, false, &x3);
  Z3ExtendSymExpr cse3(32, true, &x4);
  Z3LogicalSymExpr bin5(&cse1, &x4, BO_EQ);
  Z3LogicalSymExpr bin6(&cse2, &x5, BO_EQ);
  Z3LogicalSymExpr bin7(&cse3, &x6, BO_EQ);
  SymConstraint sc2(&bin4, true);
  SymConstraint sc3(&bin5, true);
  SymConstraint sc4(&bin6, true);
  SymConstraint sc5(&bin7, true);
  adapter->assertSymConstraint(sc2);
  adapter->assertSymConstraint(sc3);
  adapter->assertSymConstraint(sc4);
  adapter->assertSymConstraint(sc5);
  if(adapter->checkSat() == SAT_Satisfiable) {
    adapter->printModel();
  }
  adapter->reset();
  
  // !(id[index1][index2] > x3)
  llvm::errs() << "// !(id[index1][index2] > x3)\n";
  ArraySymbol id(7, 32, 2);
  Z3Symbol index1(8, SymExpr::getArrayIndexTypeBitSize(), true);
  Z3Symbol index2(9, SymExpr::getArrayIndexTypeBitSize(), true);
  Z3ElemSymExpr ese1(&id, &index1, true);
  Z3ElemSymExpr ese2(&ese1, &index2, true);
  Z3LogicalSymExpr bin8(&ese2, &x3, BO_UGE);
  SymConstraint sc6(&bin8, false);
  adapter->assertSymConstraint(sc6);
  if(adapter->checkSat() == SAT_Satisfiable) {
    adapter->printModel();
  }
  adapter->reset();
}

void testMemLeak() {
  llvm::errs() << "// Test memory leak . . .\n";
  for(int i = 0; i < 10000; i ++)
    testZ3Adapter();
}
