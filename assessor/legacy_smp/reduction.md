# T53.10 assessor analysis

For n=qp+r and i<p, q*i+min(i,r) lies within n. Adjacent endpoints coincide,
the final endpoint is n, and lengths differ by at most one. Check n<p and
SIZE_MAX range arithmetic independently of allocating real input objects.
The explicit atomic visit ledger catches overlap and omission even when sums
cancel. It is real verification overhead and must be included consistently.

Only the claiming worker writes a slot. Publication uses a release terminal
state; finish acquire-loads every state before any plain result access. It
returns INCOMPLETE while any worker is unstarted/running, even if another has
already failed. A failed partial never becomes a global accepted value.
Cleanup requires joining all callers; logical completion does not authorize
reuse while a callback can still access storage. The native injected launch
failure wakes created threads in abort mode and joins them without visits.

Unsigned addition checks against MAX-b; signed addition uses sign-separated
prechecks whose subtractions are representable. Both local and combination
stages can refuse. The selected order may refuse even if a different mathematical
association would fit. Do not repair this by signed wrap or silently relabel
modulo arithmetic as the original checked sum.

For bytes, adjacent summaries compose as (H1*P2+H2,P1*P2) modulo2^64. This
follows from concatenating the original polynomial, so it is associative and
preserves order. It is not commutative. Histograms instead combine binwise and
discard order; the two results deliberately assess different information.

The binary64 fixture's exact sum2 is independently computable with integers.
Plain p2 combines rounded1e16 and-1e16, giving0; p1/p4 fixed order gives1.
Neumaier's additional correction recovers2 here. Repeated equality establishes
this order's reproducibility, not general accuracy or all-rounding-mode safety.
No fast-math or guest floating support is claimed.

The native one-worker run performs identical arithmetic, ledger work and the
same histogram pass as larger worker counts. Wall/process CPU boundaries include
launch, gated execution, join and combination. Thread CPU clocks measure each
worker body. Packed four-bin rows can share a selected64-byte cache line while
128-byte row spacing separates them under that assumption; neither the layout
nor WSL timing proves physical cache topology, coherence traffic or a speedup.

The retained dynamic257-job producer is executed unchanged as a comparison.
Its fetch-add cursor assigns jobs dynamically, while static ranges assign
known intervals. Its fixed bounded input and all-CPU pin mask are narrower than
the new general checked/ownership contract. Do not silently bless its partial
thread-creation failures or claim static per-index evidence from its sum alone.

The actual guest's all-workers-on-BSP control computes a valid total while
failing recorded CPU identity. The reference records sparse APIC identity,
per-worker intervals and completed counts before accepting. Local virtual TSC
samples are never labeled physical speedup. Evaluate all three original leaf
criteria and delayed variants separately; external physical/learner gates remain.
