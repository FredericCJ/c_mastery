# Index component dependency boundary

The component selects profile C, not the strict E/F whole-image variant.
Its four implementation translation units use only `index.h`, which includes
`stdint.h` and `stddef.h`. They call no allocation, sorting, search, filesystem,
threading, device or standard C-library API in source. All storage is actual
typed caller-owned objects or bounded automatic scratch. No callbacks or hidden
input borrows exist. The hosted public driver, tests and benchmark select
profile A and may use libc; Python models and sanitizers are host verification.

The compiler-support allowlist for the measured uninstrumented component objects
is `memcpy` for structure-copy lowering and `__aeabi_uldivmod` for unsigned64-bit
remainder lowering on the selected Arm EABI. These names are requirements on
the integrating environment, not linked implementations supplied by this index
package. All other undefined symbols are rejected by the component-object audit.
The reference does not call them explicitly to perform an index mechanism.

Observed GCC15 native O3 objects have no undefined symbols; Clang21 O3 emits
`memcpy` in Fenwick and suffix construction. Cortex-M4 GCC O2 emits `memcpy` in
those same objects and `__aeabi_uldivmod` in segment arithmetic. The measurement
receipt pins the exact compiler output; future toolchains must rerun the audit.
Do not infer no-runtime closure from successful `-ffreestanding` compilation.

The hosted harness may resolve memory copying through its declared C runtime;
the component's compiler-support requirement remains visible. A selected target
integration must supply an inspected memory helper and an inspected ABI-correct
division helper, deliberately link and identify a permitted compiler runtime,
or change the arithmetic/copy lowering and revalidate it. A strict E/F image
must use original audited support and prohibit the external runtime. No such
whole target image, startup, flash execution or physical timing is accepted by
this hosted/component exercise. This is a bounded object-cost portability
observation, not an unavailable external prerequisite or a completed target port.

AddressSanitizer/UndefinedBehaviorSanitizer add a separate set of instrumentation
calls and startup objects. Their hosted reports cannot stand in for this
uninstrumented allowlist. The benchmark's libc, clock and printing costs are
outside the component; retained raw timing samples do not prove worst-case
execution time. Scratch includes all recursive frames and local arrays, and
`.su` reports must be read together with the call graph rather than summed
as if all functions were simultaneously live.
