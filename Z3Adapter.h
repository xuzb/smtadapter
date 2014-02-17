#ifndef LASER_Z3_ADAPTER_H	// -*- C++ -*-
#define LASER_Z3_ADAPTER_H
#include "Solver.h"
#include "Symbol.h"
#include "lib/z3/src/api/c++/z3++.h"
#include <map>
#include <string>
#include <vector>

namespace solver {


class Z3Adapter : public SolverAdapter {
public:
  Z3Adapter(unsigned t);

  //override
  virtual SolverResult checkSat();
  virtual void assertSymConstraint(const SymConstraint &sc);
  
  z3::expr genZ3Expr(const SymExpr *cond);
  void printModel();

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
