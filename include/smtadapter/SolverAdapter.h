#ifndef SMTADAPTER_SOLVER_ADAPTER_H    // -*- C++ -*-
#define SMTADAPTER_SOLVER_ADAPTER_H

namespace smt {

class SymExpr;

enum SolverResult {
  SAT_Satisfiable = 0,
  SAT_Unsatisfiable,
  SAT_Timeout,
  SAT_Undetermined // Used by solver internally to trigger restart.
};

struct SymConstraint {
  // FIXME: use pointer's bit for assumption.
  const SymExpr *cond;
  bool assumption;

  SymConstraint(const SymExpr *e, bool a)
    : cond(e), assumption(a) {}

  bool operator==(const SymConstraint &rhs) const {
    return cond == rhs.cond && assumption == rhs.assumption;
  }  
};

class SolverAdapter {
public:
  // Check the current asserted fomulars.
  virtual SolverResult checkSat() = 0;
  virtual void assertSymConstraint(const SymConstraint &sc) = 0;
  virtual void printModel() = 0;
  virtual void reset() = 0;
};

SolverAdapter *CreateZ3SolverAdapter();

}

#endif
