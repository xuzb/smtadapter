#include "smtadapter/SolverAdapter.h"
#include "Z3Adapter.h"

namespace smt {
SolverAdapter *CreateZ3SolverAdapter() {
  return new Z3Adapter();
}
}
