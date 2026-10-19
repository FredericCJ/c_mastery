# T46.11 assessor analysis

<a id="11a"></a>
## Exact bytes and native layout

The independent encoding is `43564d31010000080000000129000000160111`.
Header: CVM1, function count1, entry0; function offset0,length8,arity0,locals0;
instructions INT=1 plus LE32 41, HOST=22 service1, RET=17. Target code never links
the source compiler. Bytewise codec operations retain this image on PPC64BE;
`Cvm` size3204 on M4 versus3216 on selected64-bit ABIs reflects native padding/
pointers, not wire format. The original CvmProgram size1076 remains a current
implementation observation, not an ISO requirement. Arbitrary other ABIs need
new records. Header/source provenance and actual emitted fixture checks defeat
merely printing a golden line. A different correct linker/entry scheme is valid
when its ABI and closure are demonstrated.

<a id="11b"></a>
## Cost table and adapter boundary

Rows are `(status,out,calls,loop,fuel,host)`:
`(0,42,1,0,29,0)`, `(10,-99,0,0,30,0)`,
`(11,-99,1,0,30,0)`, `(7,-99,0,0,0,1)`,
`(7,-99,1,0,0,0)`, `(0,42,1,256,29,0)`.
Derive these by charging INT, HOST, RET. HOST_BUDGET10 occurs before callback;
HOST_ERROR11 occurs after charging/calling. Fuel1 stops before HOST; fuel2
allows HOST but cannot fetch RET. Out is published only on success. Callback
iterations are native work and do not consume VM fuel. The exact callback
successor changes only successful42 to43. Every error sentinel, call count and
budget remains the same. Printing43 without executing the same core/image fails.

The independent kernel receives the frozen bytecode through its actual archive
syscall and reports the same six rows. Four baseline case26 receipts cover
GCC/ClangO0/O2. The frozen provider also records scratch/truncation/one-page
failure variants; do not pretend those are unchanged baseline runs. The consumer
checks source, every artifact, returncode, raw transcript and both emitted and
archived bytes. It reuses actual earlier author executions after checking their
identity, rather than calling a model or labelling the consumer as a fresh boot.
Probe/core are not reentrant; adapters have exclusive ownership. T48 handles
the actual own-kernel process/stack boundary; T46.12 handles async lifetime.

<a id="11c"></a>
## Closure, startup and the observed LTO defect

The strict M4 object set is core, probe, public driver, original byte runtime,
bounded diagnostic adapter and reset assembly. There are no runtime archives.
64-bit variable division adds `__aeabi_uldivmod`; reject before linking, including
dead objects. This negative is an actual compiling dependency specimen, not a
semantic mutant. Its denominator is not executed and makes no UB golden claim.

The LTO pilot's pre-link imports were explained, but final lowering introduced
memcpy/memset after those original definitions had been discarded/internalized.
The unrooted control reproduces a link failure. Explicit linker undefined roots
retain the original definitions; final symbols/maps and LTO `.su` records must
still pass. All C objects participate in LTO. Adding `-lgcc`, removing LTO or
ignoring the final map does not satisfy this changed condition. A different
legitimate visibility/retention fix is acceptable if it preserves the strict
closure and is independently audited.

The initialized volatile word makes `.data` nonempty. Filling all BSS before
clearing defeats the zero-emulator-memory vacuity. Skip-clear image really builds
and executes, then fails `VX_STARTUP_FAIL`; this is the semantic control. Vectors,
copy/clear assembly, ranges, actual values and final image all matter. A check of
source flags alone earns no closure credit.

Stack audit sums live file-qualified static frames plus1024 and rejects dynamic
frames. This is conservative only because the inspected core uses explicit
frames/worklists, the callback is bounded/nonrecursive, and startup masks IRQs.
VM recursive fixture fails at16 explicit frames; it does not add recursive native
frames. Arbitrary future callbacks, enabled interrupt nesting, or a new compiler
invalidate this argument. A successful link does not establish stack safety or
physical timing. Final reports record actual bounds, rather than this analysis
inventing a measurement.

## Discrimination and limitations

The independent comparator deliberately rejects wrong value, callback count,
fuel, wire byte and omitted-run count. Actual learner stub still compiles and
fails. Native sanitizer runs inspect only these defined cases. PPC64 is QEMU
user emulation, M4 is system emulation, and kernel is original disposable teaching
guest. All are author-side observations; reviewer independence, delayed learner
transfer and board electrical/timing behavior remain unperformed. Look for the
false explanations “same bytecode implies same service,” “freestanding implies
no helpers,” “LTO objects are final code,” and “fuel bounds callback time.”
