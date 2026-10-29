# Aliasing questions: analysis only

These fragments are not runnable tests. Supply the containing object, effective
type, lifetime, access type, edition/extension and missing assumptions before
classifying them. A compiler result cannot repair a false premise.

1. `int x=17; float *p=(float *)(void *)&x; return *p;`: analyze alignment
   independently of the incompatible access. Would disabling one optimization
   make the access an ISO C guarantee?
2. `unsigned char bytes[sizeof(struct fd_value)];` receives a live value's representation.
   Compare accessing those declared bytes through `struct fd_value *` with copying the
   same bytes into actual allocated storage and with assigning actual fields.
3. A restricted XOR loop receives `[base,base+4)` and `[base+2,base+6)`.
   Compare it with two identical read-only inputs and a disjoint output. Explain
   why byte overlap alone is not the complete formal definition of `restrict`.
4. A pointer is saved, its allocation is freed, and a later allocation happens
   to have the same printed address. May the saved pointer be evaluated for
   comparison or used to access the new allocation? Separate that example from
   a live owner's checked integer generation token.
5. An integer loaded from a file happens to equal a live pointer's numerical
   representation. Compare C11 draft guarantees with N2219v3's wildcard proposal,
   N2263v4's empty/single provenance and the selected compiler's documentation.
   Do not implement a fictitious compiler `-fno-provenance` option.
6. `union U { float f; uint32_t u; };`: compare direct union-member access,
   casting an unrelated `float *` to `uint32_t *`, and the explicitly named
   `may_alias` extension. Identify representation/trap/alignment assumptions.
7. Given only an arbitrary `int *`, subtract `offsetof(struct os_container,member)` and
   cast to `struct os_container *`. Explain why the actual `os_member` API instead
   retains the live enclosing owner and proves the member location.

The original intentionally flawed generation-wrap control is executable: it
changes a logical value returned through a live pool. It does not dereference
freed memory. All other invalid accesses above remain analysis-only.
