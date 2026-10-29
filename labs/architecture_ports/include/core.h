#ifndef B06_PO_H
#define B06_PO_H
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>
struct po_context { uint64_t saved[32]; unsigned arch; };
struct po_status { unsigned sie,spie,spp; };
int po_stack(uint64_t top, uint64_t bytes, uint64_t *out);
int po_saved(unsigned arch, unsigned reg);
int po_arg(unsigned index, unsigned *reg, unsigned *offset);
int po_save(struct po_context *dst, unsigned arch, const uint64_t *regs);
int po_restore(const struct po_context *src, unsigned arch, uint64_t *regs);
int po_sign12(uint32_t raw);
int po_branch(int64_t delta, unsigned bits, unsigned align);
int po_trap(struct po_status *s, unsigned from_supervisor);
int po_return(struct po_status *s);
int po_access(unsigned user, unsigned supervisor, unsigned sum, unsigned mxr, unsigned r, unsigned w, unsigned x, unsigned op);
int po_closure(unsigned required, unsigned provided);
#endif
