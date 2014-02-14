#include "Z3Adapter.h"
#include <string>
#include <sstream>

using namespace solver;

Z3Adapter::Z3Adapter(unsigned t) : s(c) {
  timeout = t;
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
  if(cond->isSymbol()) {
    const Symbol *sym = dynamic_cast<const Symbol *>(cond);
    std::string id = sym->getSymName();
    std::map<std::string, z3::expr>::iterator it = decls.find(id);
    if(it != decls.end()) {
      return (it->second);
    }
    else {
      z3::expr e(c, c.bv_const(id.c_str(), sym->getTypeBitSize()));
      decls.insert(std::pair<std::string, z3::expr>(id, e));
      return e;
    }
  } else if(cond->isElemSymExpr()) {
    const ElemSymExpr *elem = dynamic_cast<const ElemSymExpr *>(cond);
    z3::expr base = genZ3Expr(elem->getBaseExpr());
    z3::expr index = genZ3Expr(elem->getIndexExpr());
    return select(base, index);
  } else if(cond->isBinSymExpr()) {
    const BinSymExpr *bin = dynamic_cast<const BinSymExpr *>(cond);
    const SymExpr *lhs = bin->getLHS();
    const SymExpr *rhs = bin->getRHS();
    //assert(lhs->getTypeBitSize() == rhs->getTypeBitSize() &&
		//lhs->isSigned() == rhs->isSigned());
    z3::expr e1 = genZ3Expr(lhs);
    z3::expr e2 = genZ3Expr(rhs);
    switch(bin->getOpcode()) {
      case BO_Mul:
        return z3::expr(c, Z3_mk_bvmul(c, e1, e2));
      case BO_Div:
        if(lhs->isSigned())
          return z3::expr(c, Z3_mk_bvsdiv(c, e1, e2));
        else return z3::expr(c, Z3_mk_bvudiv(c, e1, e2));
      case BO_Rem:
        if(lhs->isSigned())
          return z3::expr(c, Z3_mk_bvsrem(c, e1, e2));
        else return z3::expr(c, Z3_mk_bvurem(c, e1, e2));
      case BO_Add:
        return z3::expr(c, Z3_mk_bvadd(c, e1, e2));
      case BO_Sub:
        return z3::expr(c, Z3_mk_bvsub(c, e1, e2));
      case BO_Shl:
        return z3::expr(c, Z3_mk_bvshl(c, e1, e2));
      case BO_Shr:
        return z3::expr(c, Z3_mk_bvlshr(c, e1, e2));
      case BO_LT:
        if(lhs->isSigned())
          return z3::expr(c, Z3_mk_bvslt(c, e1, e2));
        else return z3::expr(c, Z3_mk_bvult(c, e1, e2));
      case BO_GT:
        if(lhs->isSigned())
          return z3::expr(c, Z3_mk_bvsgt(c, e1, e2));
        else return z3::expr(c, Z3_mk_bvugt(c, e1, e2));
      case BO_LE:
        if(lhs->isSigned())
          return z3::expr(c, Z3_mk_bvsle(c, e1, e2));
        else return z3::expr(c, Z3_mk_bvule(c, e1, e2));
      case BO_GE:
        if(lhs->isSigned())
          return z3::expr(c, Z3_mk_bvsge(c, e1, e2));
        else return z3::expr(c, Z3_mk_bvuge(c, e1, e2));
      case BO_EQ:
        return e1 == e2;
      case BO_NE:
        return e1 != e2;
      case BO_And:
        return z3::expr(c, Z3_mk_bvand(c, e1, e2));
      case BO_Xor:
        return z3::expr(c, Z3_mk_bvxor(c, e1, e2));
      case BO_Or:
        return z3::expr(c, Z3_mk_bvor(c, e1, e2));
      case BO_LAnd:
        return e1 && e2;
      case BO_LOr:
        return e1 || e2;
    }
  } else if(cond->isUnarySymExpr()) {
    const UnarySymExpr *un = dynamic_cast<const UnarySymExpr *>(cond);
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
  } else if(cond->isCastSymExpr()) {
    const CastSymExpr *ce = dynamic_cast<const CastSymExpr *>(cond);
    const SymExpr *operand = ce->getOperand();
    z3::expr e = genZ3Expr(operand);
    int sizeDiff = operand->getTypeBitSize() - ce->getTypeBitSize();
    int newBitSize = ce->getTypeBitSize();
    bool oldSigned = operand->isSigned();
    if(sizeDiff > 0) {
      // Extract
      return z3::expr(c, Z3_mk_extract(c, newBitSize - 1, 0, e));
    } else {
      if(oldSigned) {
        // Sign-extend
        return z3::expr(c, Z3_mk_sign_ext(c, -1 * sizeDiff, e));
      } else {
        return z3::expr(c, Z3_mk_zero_ext(c, -1 * sizeDiff, e));
      }
    }
  } else if(cond->isConstExpr()) {
    const ConstExpr *ce = dynamic_cast<const ConstExpr *>(cond);
    z3::expr e = genZ3Const(ce);
    return z3::expr(c, Z3_mk_int2bv(c, ce->getTypeBitSize(), e));
  }
}

z3::expr Z3Adapter::genZ3Const(const ConstExpr *ce) {
  if (ce->isSigned())
    return c.int_val((__int64)ce->getValue());
  else return c.int_val((__uint64)ce->getValue());
}
