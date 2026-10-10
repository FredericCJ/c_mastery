# Staged hints

First write your prediction and invariant. Reveal one column only when your own
counterexample does not resolve the failure. Record hints used; delayed transfer
must use new inputs without them. Do not read assessor source during the attempt.

| Mechanism | Stage 1: locate the boundary | Stage 2: state the invariant | Stage 3: implementation direction |
|---|---|---|---|
| Ring | Does equal head/tail mean empty or full? | Live sequence occupies count positions from head modulo capacity | Derive insertion offset from head+count; reject before writing |
| Hash | What happens when the matching key lies after a deleted slot? | Empty ends the probe; tombstone preserves reachability | Remember first reusable slot while continuing the duplicate search |
| AVL | Which ancestor's stored height first becomes wrong? | Each returned subtree root has correct height and balance | Return possibly rotated root from recursive mutation; reuse dead typed slots |
| Graph | Can mathematical infinity be added as a machine integer? | Settled distances are final only under nonnegative edge premises | Keep a local result array; check candidate subtraction bound before addition and publication |
| Lower bound | What does equality do to hi? | `[0,lo)` is less than key; `[hi,n)` is at least key | Use half-open range and `lo+(hi-lo)/2`; unsigned branch in each ISA |
| Stable insertion | Which record is temporarily absent from the array? | Saved key+tag plus shifted suffix preserve the multiset | Move all eight bytes; shift only strictly larger unsigned keys |
| Heap | Which one edge set may still violate order? | Children of the current hole are heaps; choose their larger root | Resolve child tie to left, compare once with parent, move hole downward |
| Match | Does a zero byte terminate this contract? | Every earlier candidate start is known not to match | Bound candidate starts using n-m after checking m≤n |
| Arena | Is the base address itself aligned? | used≤capacity and padding+bytes fits remaining space | Derive padding from actual address; subtract before adding sizes |
| Pool | Why can a reused slot number identify the wrong object? | A live handle matches both slot and current generation | Increment generation on release; retire at maximum instead of wrapping |
| Q7 | When does rounding occur relative to saturation? | At most16 widened products fit the declared accumulator | Round absolute magnitude with half-unit bias, restore sign, then clamp |
| ALU/cache | Is signed overflow the same as carry-out? | Bit carry and signed representability are different outputs | Compare carry into/out of sign bit; use a separate LRU shadow to classify misses |
| ABI | Can the nested test wrapper hide the corruption? | Every preserved register seen by the probe survives the actual kernel | Use caller scratch for the wrapper's temporary alignment check; do not restore the tested register before checking it |

For each repair, explain the smallest triggering input, the observed failure,
the violated invariant and why the patch restores it. Then change one dimension:
duplicate distribution, high-bit values, capacity, graph connectivity, address
alignment or failure point. Report unsupported dimensions directly.
