#include "smtadapter/SolverContext.h"
#include "Z3Adapter.h"
#include <cstdlib>
#include <string>
#include <sstream>

using namespace smt;

Z3Adapter::Z3Adapter(SolverContext &sc)
: SolverAdapter(sc), timeout(5000), c(), s(c) {
  z3::params p(c);
  p.set(":timeout", timeout);
  s.set(p);
}

Z3Adapter::Z3Adapter(SolverContext &sc, unsigned t)
: SolverAdapter(sc), timeout(t), c(), s(c) {
  z3::params p(c);
  p.set(":timeout", timeout);
  s.set(p);
}

SolverResult Z3Adapter::checkSat() {
  int result = s.check();
  if(result == z3::unsat)
    return SAT_Unsatisfiable;
  if(result == z3::sat)
    return SAT_Satisfiable;
  if(result == z3::unknown) {
    std::string reason = s.reason_unknown();
    if(reason == "timeout")
      return SAT_Timeout;
    else
      return SAT_Undetermined;
  }
  assert(0 && "Unknown z3 result.");
  return SAT_Undetermined;
}

void Z3Adapter::reset() {
  s.reset();
  decls.clear();
}

void Z3Adapter::printModel() {
  z3::model m = s.get_model();
  std::ostringstream oss;
  oss << m;
  std::cout << oss.str() << "\n";
}

void Z3Adapter::assertSymConstraint(const SymConstraint &sc) {
  z3::expr cond = genZ3Expr(sc.cond);
  if(sc.assumption)
    s.add(cond);
  else s.add(!cond);
}

z3::expr Z3Adapter::genZ3Expr(const SymExpr *cond) {
  switch (cond->getKind()) {
  default: {
    assert(0 && "Unprocessed z3 expr.");
    exit(1);
  }
  case SymExpr::S_ScalarSymbol: {
    const Symbol *sym = static_cast<const Symbol *>(cond);
    unsigned id = sym->getSymbolID();
    std::map<unsigned, z3::expr>::iterator it = decls.find(id);
    if(it != decls.end()) {
      return (it->second);
    } else {
      z3::expr e(c, c.bv_const(sym->getSymName().c_str(),
                               sym->getTypeSizeInBits(ctx)));
      decls.insert(std::pair<unsigned, z3::expr>(id, e));
      return e;
    }
  }
  case SymExpr::S_RegionSymbol: {
    const RegionSymbol *asym = static_cast<const RegionSymbol *>(cond);
    unsigned id = asym->getSymbolID();
    std::map<unsigned, z3::expr>::iterator it = decls.find(id);
    if (it != decls.end()) {
      return it->second;
    } else {
      unsigned indexSize = ctx.getArrayIndexTypeSizeInBits();
      unsigned elemSize = asym->getElementTypeSizeInBits(ctx);
      unsigned nDim = asym->getNumberDimension(ctx);
      z3::sort indexSort = c.bv_sort(indexSize);
      z3::sort valueSort = c.bv_sort(elemSize);
      for (int i = 0; i < nDim; ++i) {
        valueSort = c.array_sort(indexSort, valueSort);
      }
      z3::expr e(c, c.constant(asym->getSymName().c_str(), valueSort));
      decls.insert(std::pair<unsigned, z3::expr>(id, e));
      return e;
    }
  }
  case SymExpr::S_ElemSymExpr: {
    const ElemSymExpr *elem = static_cast<const ElemSymExpr *>(cond);
    z3::expr base = genZ3Expr(elem->getBaseExpr());
    z3::expr index = genZ3Expr(elem->getIndexExpr());
    return select(base, index);
  }
  case SymExpr::S_ArithSymExpr: {
    const ArithSymExpr *bin = static_cast<const ArithSymExpr *>(cond);
    const SymExpr *lhs = bin->getLHS();
    const SymExpr *rhs = bin->getRHS();

    z3::expr e1 = genZ3Expr(lhs);
    z3::expr e2 = genZ3Expr(rhs);
    switch(bin->getOpcode()) {
    default:
      assert(0 && "Unprocessed arith opcode.");
    case BO_Mul:
      return z3::expr(c, Z3_mk_bvmul(c, e1, e2));
    case BO_SDiv:
      return z3::expr(c, Z3_mk_bvsdiv(c, e1, e2));
    case BO_UDiv:
      return z3::expr(c, Z3_mk_bvudiv(c, e1, e2));
    case BO_SRem:
      return z3::expr(c, Z3_mk_bvsrem(c, e1, e2));
    case BO_URem:
      return z3::expr(c, Z3_mk_bvurem(c, e1, e2));
    case BO_Add:
      return z3::expr(c, Z3_mk_bvadd(c, e1, e2));
    case BO_Sub:
      return z3::expr(c, Z3_mk_bvsub(c, e1, e2));
    case BO_Shl:
      return z3::expr(c, Z3_mk_bvshl(c, e1, e2));
    case BO_Shr:
      return z3::expr(c, Z3_mk_bvlshr(c, e1, e2));
    case BO_And:
      return z3::expr(c, Z3_mk_bvand(c, e1, e2));
    case BO_Xor:
      return z3::expr(c, Z3_mk_bvxor(c, e1, e2));
    case BO_Or:
      return z3::expr(c, Z3_mk_bvor(c, e1, e2));
    }
  }
  case SymExpr::S_LogicalSymExpr: {
    const LogicalSymExpr *bin = static_cast<const LogicalSymExpr *>(cond);
    const SymExpr *lhs = bin->getLHS();
    const SymExpr *rhs = bin->getRHS();

    z3::expr e1 = genZ3Expr(lhs);
    z3::expr e2 = genZ3Expr(rhs);
    switch(bin->getOpcode()) {
    default:
      assert(0 && "Unprocessed logical opcode");
    case BO_SLT:
      return z3::expr(c, Z3_mk_bvslt(c, e1, e2));
    case BO_ULT:
      return z3::expr(c, Z3_mk_bvult(c, e1, e2));
    case BO_SGT:
      return z3::expr(c, Z3_mk_bvsgt(c, e1, e2));
    case BO_UGT:
      return z3::expr(c, Z3_mk_bvugt(c, e1, e2));
    case BO_SLE:
      return z3::expr(c, Z3_mk_bvsle(c, e1, e2));
    case BO_ULE:
      return z3::expr(c, Z3_mk_bvule(c, e1, e2));
    case BO_SGE:
      return z3::expr(c, Z3_mk_bvsge(c, e1, e2));
    case BO_UGE:
      return z3::expr(c, Z3_mk_bvuge(c, e1, e2));
    case BO_EQ:
      return e1 == e2;
    case BO_NE:
      return e1 != e2;
    case BO_LAnd:
      return e1 && e2;
    case BO_LOr:
      return e1 || e2;
    }
  }
  case SymExpr::S_UnarySymExpr: {
    const UnarySymExpr *un = static_cast<const UnarySymExpr *>(cond);
    const SymExpr *operand = un->getOperand();
    z3::expr e = genZ3Expr(operand);
    switch(un->getUnaryOpcode()) {
      case UO_Minus:
        return z3::expr(c, Z3_mk_bvneg(c, e));
      case UO_Not:
        return ~e;
      case UO_LNot:
        return !e;
    }
  }
  case SymExpr::S_TruncSymExpr: {
    const TruncSymExpr *ce = static_cast<const TruncSymExpr *>(cond);
    const SymExpr *operand = ce->getOperand();
    z3::expr e = genZ3Expr(operand);
    
    return z3::expr(c, Z3_mk_extract(c, ce->getTypeSizeInBits(ctx) - 1, 0, e));
  }
  case SymExpr::S_ExtendSymExpr: {
    const ExtendSymExpr *ce = static_cast<const ExtendSymExpr *>(cond);
    const SymExpr *operand = ce->getOperand();
    z3::expr e = genZ3Expr(operand);
    int newBitSize = ce->getTypeSizeInBits(ctx);

    // LogicalSymExpr is a Boolean expr that shoud be evaluated by using ite.
    if (LogicalSymExpr::classof(operand)) {
      assert(e.is_bool());
      z3::expr trueBV = c.bv_val(1, newBitSize);
      z3::expr falseBV = c.bv_val(0, newBitSize);
      return z3::expr(c, Z3_mk_ite(c, e, trueBV, falseBV));
    }

    int oldBitSize = operand->getTypeSizeInBits(ctx);
    int sizeDiff = newBitSize - oldBitSize;
    assert(sizeDiff > 0 && "The targe type size should be greater than old type size.");

    if(ce->isSignedExt()) {
      return z3::expr(c, Z3_mk_sign_ext(c, sizeDiff, e));
    } else {
      return z3::expr(c, Z3_mk_zero_ext(c, sizeDiff, e));
    }
  }
  case SymExpr::S_ConstExpr: {
    const ConstExpr *ce = static_cast<const ConstExpr *>(cond);
    return genZ3Const(ce);
  }
  }
}

z3::expr Z3Adapter::genZ3Const(const ConstExpr *ce) {
  unsigned sz = ce->getTypeSizeInBits(ctx);
  if (ce->isSigned())
    return c.bv_val((__int64)ce->getValue(), sz);
  else return c.bv_val((__uint64)ce->getValue(), sz);
}
