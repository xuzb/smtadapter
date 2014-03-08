#include "smtadapter/SolverAdapter.h"
#include "Z3Adapter.h"

namespace solver {
SolverAdapter *CreateZ3SolverAdapter() {
  return new Z3Adapter();
}
}
