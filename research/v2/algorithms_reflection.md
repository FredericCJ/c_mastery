# Research reflection: executable algorithm companions

The existing projects are much broader than a small demonstration suite. A passing
bounded kernel cannot accept an entire container library, allocator family,
numerical analysis sequence or computer architecture track. Each mapping therefore
names the tested mechanism and the remaining project work.

Existing accepted inspections supply the relevant insertion invariant, heap
repair, open-address tombstone, BFS, naive byte matching, arena/interface and
scalar ABI contracts. Their edition hashes and exact scope are reused. Metadata
in the reference graphs remains discovery evidence. The held CLRS and Hanson
documents are not newly or wholly read by this task.

The technical uncertainty lies at translations: mathematical infinity to checked
integers; unsigned comparisons in two instruction sets; deleting hash entries;
stable whole-record movement; fixed-pool reuse; scalar PCS preservation; and RAM
operation counts versus cycles. Test those boundaries directly. Keep modeling,
functional emulation, hosted sanitizers and physical evidence separate.

Reflection changed the implementation scope: provide three complete original
assembly kernels in both required ISAs and independent ABI probes, instead of
claiming compiler output is assembly authorship. Prefer bounded, exhaustively
checked representations to a generic framework that hides ownership decisions.
