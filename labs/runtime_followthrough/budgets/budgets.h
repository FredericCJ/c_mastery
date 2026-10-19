#ifndef CMASTERY_VM_BUDGETS_H
#define CMASTERY_VM_BUDGETS_H
#include "execution.h"
typedef struct {
  uint32_t fuel, gc, host, allocations, collections;
  int status;
} VlReport;
/* Borrow a stopped initialized VM/program. Callback obeys the original CVM
 * contract and must not modify budgets/counters. Disjoint live outputs. Report
 * is published on every execution result; value only on CVM_OK. Counts are
 * differences in semantic units, never elapsed cycles. No counter wrap during
 * this bounded call. The caller reinitializes before independent replay. */
int vl_run(Cvm *, const CvmProgram *, CvmValue *, VlReport *);
/* One actual Vs step; pending and sticky outcomes do not mean native provider
 * quiescence. No callback is invoked by this API. Value only on VS_DONE. */
int vl_step(Vs *, CvmValue *, VlReport *);
#endif
