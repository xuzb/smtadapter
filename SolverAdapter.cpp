#include "smtadapter/SolverAdapter.h"
#include "Z3Adapter.h"

namespace smt {
SolverAdapter *CreateZ3SolverAdapter(SolverContext &ctx) {
  return new Z3Adapter(ctx);
}
}
