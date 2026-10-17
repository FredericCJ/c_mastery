# Required learner integration adapters

The public kernels have no hidden library dependency. PR-T74-11 must consume
the actual completed `runtime.vm`, `memory.pool`, `testing.faults` and
`measurement.capture` artifacts through explicit, source-bound adapters.

Map a CvmValue to an explicit OR tagged value only after checking type and live
generation; never reinterpret the baseline struct or truncate its pointer.
Translate the VM root stack, operand stack and each initialized live local to
ORFrame slots, preserving all copies and their base/offset information. Document
which VM instructions can allocate or reach a safepoint. Root temporary C
references before a collecting allocation. Existing T46 nonmoving mark/sweep
is retained as a differential oracle; it is not silently converted in place.

Supply memory.pool storage with an exclusive ownership interval for each cell
and a handle-generation policy. Release/retire only after collector reachability
and outstanding raw borrows permit it. Fixed identity slots in the anchor are
never a substitute for a reuse protocol. Fault injection must cover admission,
promotion exhaustion, interning resize and finalizer-owned resource failure.
The measurement adapter records traces, operation counts, memory peaks and
equivalent correctness contracts before collecting timing distributions.

Write ADAPTER_EVIDENCE.md with exact producer source revisions, export contracts,
conversion code, lifetime diagrams and composition tests. Export
`rt.object_model` only after representation/intern/shape/cache checks, and
`rt.incremental_gc` only after root/store barriers, collection and VM-root
integration are assessed. These adapters are learner work; they are not
implemented or automatically copied from assessor artifacts by this scaffold.
