# T19 dispatch under equivalent transport contracts

Required PR-T19-04 T19.10.a/b/c; preserve original AX identifiers, families
A/B/C/D/F/H/I/J and seven-day changed tasks. Import the existing `sf_start/pump`
producer and its T19.08 prerequisites unchanged. Implement the three quota
backends, direct switch adapter, operation-table adapter and a fixed-one-byte
specialization. C17 hosted GCC/Clang, O0/O2, with/without LTO; Linux ELF shared
objects are an explicit ABI/loader variant. This packet does not claim a strict
freestanding closure or external network performance. All bytes are synthetic
and written only to original in-memory teaching sinks.

## Preserve behavior before comparing cost

The provider stages `[length,payload,checksum]`, payload0..64, in a66-byte frame.
Keep its admission, progress, retry, cancellation, generation and output contracts.
Each sink has66 bytes, accepted length, calls, offered bytes and an injected mode.
Backend one/four/all accepts min(remaining,1/4/66). Validate pointers, available
capacity and counter overflow before mutation. Mode1 retries once with-2; mode2
reports backpressure0; mode3 reports terminal error-1; mode4 deliberately returns
too large a count without touching data. The caller detects this contract violation;
it cannot undo a malicious callback's prior memory accesses in a general case.

Direct dispatch switches on kind0..2 to an exactly typed backend. Erased dispatch
uses a borrowed immutable `struct ad_ops` with version1, kind and exact function pointer.
Validate descriptor metadata before calling, but recognize that metadata cannot
prove an arbitrary address is live or that an arbitrary function honors the contract.
The descriptor and sink outlive every synchronous callback; there is no retention,
reentrancy, allocation or hidden thread. Failed init preserves output. All backend
implementations share the same byte/capacity validation routine. Direct switch
and table metadata admission differ in representation while enforcing the same
public transport behavior for valid descriptors/kinds.

The stream itself has a common `sf_write_fn` callback in every variant. The experiment
compares dispatch *inside that callback*, not a hypothetical stream rewritten to
eliminate all function pointers. Direct source syntax is not an assertion that the
compiler will emit a direct call. Keep this scope in every cost conclusion.

Test455 complete frames:65 lengths times three direct backends, three table
backends and the fixed specialization. Independently reconstruct every frame and
checksum. If total frame bytes=t and quota=q, callback count k=ceil(t/q), and
offered bytes=k*t-q*k*(k-1)/2. Compare these counts and all bytes, then stable
DONE without another callback. Exercise12 injected progress/error paths across
direct/table/fixed forms, malformed descriptors, unknown kinds, null nonempty
span and full sink. Both byte corruption and wrong-backend dispatch must compile
and be rejected semantically; learner stubs must compile and fail conformance.

## Inspect actual linked code and the visibility boundary

Build16 cost executables: two compilers, O0/O2, LTO on/off and same-link versus
separate shared backend. The shared library is deliberately compiled without LTO,
then loaded by the ELF dynamic loader at process startup. Retain its NEEDED entry,
symbols, sections and disassembly along with the executable. This is actual shared
library loading, not runtime hot unloading or an ISO C module facility. The library
remains loaded throughout all borrowed operation-table uses. Do not infer safe
unload while a descriptor, function pointer or callback remains live.

Annotate the stream call, adapter call/switch, backend copy and failure branches;
distinguish CRT/PLT calls from mechanism calls. Explain any inlining or
devirtualization by this build's visibility. Actual indirect-call instruction lines
are inspection aids, not hardware executed-branch counters. The retained original
provider uses compact formatting; its misleading-indentation warning is disabled
for these builds without changing that provider's behavior or other diagnostics.

## Measure a declared workload

For each build, run direct/table with homogeneous backend0, alternating0/1 and
cycling0/1/2; run fixed only with backend0. Compare payload0 with64. Each of224
cells executes one warmup and nine recorded batches,3000 frames per batch. Only
the selected representation is initialized. Each frame is fully verified during
the timed region. Record raw monotonic nanoseconds, min/median/max, calls, offered
and accepted bytes, checksum, actual direct/erased/fixed/table sizes and zero
target allocations. Frame construction copies n logical payload bytes and the
sink accepts n+2; compiler-generated structure assignment or vectorization may
have different physical traffic. Count those logical operations separately from
physical memory traffic and shared harness costs.

These are full correctness-checked frame timings, not isolated call latency. Host
scheduling, other processes, order and thermal state remain uncontrolled; retain
variability and do not claim a portable speedup or latency bound. Hardware counters
are not collected by this campaign. Do not invent branch misses from elapsed time
or assembly. A64-byte payload increases framing, verification and copying work;
derive its changed proportion rather than assuming it dominates on every backend.

## A removal proposal has a narrower contract

The supplied fixed specialization takes only `struct ad_sink` and calls backend-one: it
removes the instance kind/table field, preserves failure/progress ownership, and
passes its65 length cases and all four injected modes. It is an eligible
simplification only for a requirement that permanently selects backend-one.
Compare actual measurements and review obligations before recommending it.
If per-instance backend variation or shared-library replacement is required, the
fixed form does not satisfy the requirement regardless of timing.

The changed third backend accepts the whole remainder. Add it to direct switch,
table and mixed traces. Count edited selection sites, representation/lifetime
obligations and duplicated validation. Prepare a decision document with measured
build/workload scope, correctness equivalence, maintenance assumptions and explicit
conditions for reversing the decision. Deleting all abstraction based on the
fastest single sample fails. Hints: count actual accepted bytes first; locate the
visibility boundary; ask when backend selection may change. Per leaf0 wrong,
1 nominal/timing-only,2 complete ordinary contract and evidence,3 independent
changed-condition analysis. Source-bound author runs do not supply independent
review, a learner's seven-day attempt or universal performance guarantees.
