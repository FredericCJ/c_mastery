# Actual CVM roots, moving values and foreign lifetimes

X-T74-011 consumes the real `cvm.h`/`cvm.c`/`compiler.c` implementation from
runtime.vm and `algo.h`/`algo.c` from memory.pool. CMake accepts explicit
`CMASTERY_RUNTIME_ROOT` and `CMASTERY_ALGORITHMS_ROOT` completed learner workspace
paths. Assessor roots are separate explicit options; they are never silently
copied into a learner export. The original CVM collector and interpreter remain
unchanged. The adapter is C17 hosted, serialized and allocation-free; its large
automatic staging objects require measured stack accounting. It is not a
concurrent collector or an interrupt-safe API.

## Representation and authority

`struct or_vm` borrows a live initialized Cvm for its whole lifetime. Initialize into
fresh control storage with `orv_init`. Callers own every referenced object and
serialize all VM/adapter access. The VM must be stopped before `orv_pause(1)`;
the flag asserts a boundary and cannot stop another thread. Resume sets it to
zero, causing collection and registration to refuse. No raw heap address may
survive a collecting operation. Old unregistered CvmValue copies can become
stale. A caller must register *every usable copy* or reconstruct it afterward.

`struct orv_derived` holds a full CvmValue base and a pair-field offset: 0, 1, or the
representable one-past offset 2. Offset 2 cannot be dereferenced. Non-reference
values require offset zero. It is a logical pair index, not a truncated native
pointer. Each registration borrows an exclusive writable ORVDerived object until
its exact ticket is unregistered. Registered objects must be mutually disjoint,
outside the Cvm and ORVM representations, and outlive registration. Outputs,
metrics and control objects must be disjoint from each other and those borrowed
objects. Public fields permit inspection and named exhaustion tests; ordinary
clients cannot forge representations, registrations or live generations.

The actual 16-cell algo_pool stores registration-directory indices. Its high
32-bit generation/low 32-bit slot tickets detect stale unregister operations;
release at the generation maximum retires the cell. It does not store raw
pointers in integers or replace the Cvm heap. Registered scalar values still
occupy a pool cell. Re-registering the same object is OR_STATE, capacity or
retirement exhaustion is OR_FULL, stale unregister is OR_MISS, and failed
admission preserves output. An initialization requires no outstanding tickets;
all previous lifetime tickets must be discarded before reinitialization.

## Collection and failure semantics

`orv_collect` stages a full Cvm copy and at most 216 values: 8 explicit roots,
64 operand values, 16 frames times 8 initialized locals, and 16 registered C
values. Uninitialized locals are neither read as values nor roots. Validate
tags, counts, initialized bits, live full-generation references, offsets and
*all used pair fields*, including unreachable pairs. The latter is stricter
than the original collector's reachable traversal. Scalar integer lookalikes
are never roots. Arena mode and a running VM refuse with OR_STATE.

Translate the graph to the actual ORGraph mark kernel. Then stable-pack reachable
pairs to the left, skipping retired destination holes. A pair staying in its
physical cell keeps its generation; a moved pair uses the destination cell's
previous generation plus one. Never transplant the source generation. A
generation-maximum live pair can stay in place; a retired hole cannot receive
another object. All old fields and all mapped copies use the same forwarding
table and destination generation. Translate each group of at most eight values
through the actual ORFrame/or_safepoint kernel; retain tags, non-reference data
and offsets. Publish VM, registered values and statistics only after all checks.

The exact work unit is one validated active root value, one validated field of
each used pair, one heap cell in the 16-cell sweep model, and one marked live
object. `work = slots + 2*used_before + 16 + reachable`. This is an observable
semantic counter, not machine instructions or elapsed cycles. Budget exhaustion
returns OR_FULL with VM, registrations' values and statistics unchanged. The
collection counter cannot wrap. Fault cuts 1,2,3,4 occur after initial validation,
after marking/budget checks, after root-map relocation, and after final staged
metadata but before publication; each returns OR_FULL unchanged. All failures
preserve caller outputs. Runtime pointer provenance and object lifetimes remain
caller obligations, not something these checks can discover.

For variable bounds H heap cells and R root copies, staging is O(H+R), and
whole-Cvm copying includes its inactive frame/operand capacity. The current
ORGraph marker searches the H-cell color array for each visited node, so worst
case time is O(H squared + R), plus bounded pair edges. Do not describe this
specific implementation as an optimal linear marker. Root frames require
ceil(R/8) calls. Explicit fixed bounds make counters representable.

## Allocating foreign boundary and every exit

`orv_call` is a new explicit allocating boundary, separate from CVM's existing
integer-only, non-reentrant CvmHost callback. It registers two C-local argument
copies before entering the callback and unregisters them on every normal status
return. If the second registration fails, undo the first without invoking the
callback. Nested orv_call and pause/resume inside that callback are OR_STATE.
The callback may call orv_pair/orv_collect and use other explicitly registered
live values. It cannot corrupt the owner, unregister hidden argument tickets,
retain pointers to those local copies, escape with longjmp or fail to return.
Callbacks return ORStatus; an error is an explicit C status, not a C++ exception.
Completed callback side effects remain after an error, while the public output
is unchanged. On success validate the returned CvmValue before publishing it.

`orv_pair` requires offset-zero argument objects. A collecting allocation requires
both exact argument objects to be registered, so every collection can update the
copies subsequently passed to actual cvm_pair. It preflights a free nonretired
cell before calling that allocator, preventing an unannounced second baseline
collection. A requested collection may succeed before allocation later reports
FULL; that successful movement is retained. Output remains unchanged on failure.
Force collection to exercise otherwise rare temporary-root paths. No existing
CvmHost contract has been widened.

The baseline interpreter can allocate at OP_PAIR and explicitly collect at
OP_GC; its original nonmoving C temporary-root protocol remains in force there.
This adapter collects at declared stopped boundaries and inside its new foreign
boundary. It does not install a poll inside every CVM dispatch instruction or
resume an arbitrary failed interpreter instruction. The test compiles and runs
an actual three-function program to CVM_HOST_ERROR, then collects the retained
frames. Keep that scope distinct from a native-code compiler's safepoint ABI.

## Concrete verification and contract imports

`object_runtime.vm_roots` checks the compiled three-frame program, duplicates in
every root class, a scalar with equal bits, two derived offsets, uninitialized
poison, all 216 slots, stopped/running states, malformed maps, full generations,
stale old handles, and exact unchanged failure snapshots. `vm_foreign` checks
allocation after movement, all failure cuts, callback errors after side effects,
nested-call refusal, full/retired real pool cells and partial registration undo.
Four separately compiled controls omit locals, transplant source generations,
omit exit cleanup, or omit successful output publication; each must fail its
own semantic assertion after successful compilation.

The independent Python oracle constructs reachability sets and output fields
for all 4^6 three-pair graphs times 8 root subsets: 32,768 cases. The C driver
also runs the unchanged cvm_collect on a separate copy. Cycles, self-edges,
duplicate edges, no roots and every root subset occur. It prints actual forwarded
identities and pair fields; the Python model does not call the moving algorithm.

`testing.faults` (PR-T32-02) supplies a model/history contract, not a generic C
fault-injection header. This adapter's explicit fault schedule, snapshots and
set oracle are its concrete extension. `measurement.capture` (PR-T43-01) supplies
counted-work and equivalent-contract discipline, not a preexisting timer API.
The VM probe records semantic work, source/build identity, monotonic elapsed
distributions, layout, code size and compiler stack reports for both moving and
unchanged nonmoving implementations on equal admitted graphs. It does not relabel
insertion-sort comparison counters as collector work. Prior learner acceptance,
delayed performance and independent assessment remain separate evidence.

The compiler experiment in `statepoints/CONTRACTS.md` executes an actual relocating
LLVM boundary with register and stack behavior. Compare its ABI-required root
map with explicit C addresses here. Both require cooperation; neither establishes
that ISO C automatically tracks arbitrary local roots. The only adversarial
targets are these original local programs with synthetic values. Lost roots and
wrong-object aliases are the harmless violation; no arbitrary execution or
privilege claim follows from them.
