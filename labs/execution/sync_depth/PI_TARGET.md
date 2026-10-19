# PI target experiment

Build `pi_m4.c` with the explicit completed CMX and PI extension imports,
the original course `support.c` memory routines, and the supplied course
`m4.ld` linker script. Run only in an owned disposable QEMU `mps2-an386`
Cortex-M4 instance. This startup/image is a dedicated teaching target; no
physical board flashing or host security setting change is part of the task.

The guest constructs L9/M7/H3's two-mutex dependency chain, changes L's base
to11 under donation, changes H's base to6, invokes the actual scheduler,
cancels both wait edges and observes restoration to11. It exercises original
CMX task records on the target ABI. It does not install a new task context
switcher, enable an interrupt producer, or establish a physical timing bound.
The separate wait-IRQ successor supplies an actual configurable IRQ experiment.

Require actual semihosted `CDPI_M4_PASS` with exit0 for the reference.
Stale-donation, missing-transitivity, overwritten-base, unsolved-extension and
unsolved-CMX controls must build/link and exit1 with `CDPI_M4_CHECK`.
`CDPI_M4_FAULT`, a timeout, arbitrary nonzero status or compiler failure is
not a semantic rejection. GCC-O0, GCC-O3 and Clang-O3 are distinct observations.

Inspect undefined symbols, the linker map and generated assembly. Explicit
original byte-memory functions/AEABI aliases are permitted because their source
is a declared dependency; do not call this a helper-free image. Reject unresolved
symbols, undeclared arithmetic/atomic helpers and libc/libgcc/libatomic or
compiler-rt archives. Host measurement code and Python oracles are not target
dependencies. Retain each exact command, image hash and output alongside the
source-bound receipt. Independent reviewer/learner/physical gates remain separate.
