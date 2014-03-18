#ifndef SMTADAPTER_Z3_ADAPTER_H	// -*- C++ -*-
#define SMTADAPTER_Z3_ADAPTER_H
#include "smtadapter/SolverAdapter.h"
#include "smtadapter/Symbol.h"
#include "lib/z3/src/api/c++/z3++.h"
#include <map>
#include <string>
#include <vector>

namespace smt {


class Z3Adapter : public SolverAdapter {
public:
  Z3Adapter();
  Z3Adapter(unsigned t);

  //override
  virtual SolverResult checkSat();
  virtual void assertSymConstraint(const SymConstraint &sc);
  
  z3::expr genZ3Expr(const SymExpr *cond);
  void printModel();
  void reset();

private:
  z3::expr genZ3Const(const ConstExpr *ce);
    
private:
  // Timeout, in milliseconds
  unsigned timeout;
  z3::context c;
  z3::solver s;
  std::map<unsigned, z3::expr> decls;

};

} // end namespace laser

#endif
