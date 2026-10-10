# Dependency and allocation contracts

Select a profile for each component and executable before implementation. The profile defines a boundary to audit. A hosted harness may have different dependencies from the reusable component it tests.

| Profile | Boundary and permitted headers | Startup and linked support | Allocation and services | Required audit |
|---|---|---|---|---|
| A: hosted C | Whole hosted program; standard headers permitted, POSIX only when declared | Platform CRT, libc and declared libraries; compiler helpers permitted | Explicit allocation allowed; record ownership, failure handling and OS use | Link command, dynamic dependencies, undefined symbols, allocator entry points |
| B: allocation-free component | Named component and declared callees; `<stddef.h>`, `<stdint.h>`, `<limits.h>` and selected interface headers; explicitly allow individual library calls | Hosted CRT normally belongs to harness; list libc/helper calls reachable from component | Caller storage only in component; separately investigate transitive allocation, TLS, lazy initialization and callbacks | Source call graph plus binary evidence and failure instrumentation; disclose opaque callees |
| C: portable component with no external C-library calls | Component only; declaration/type headers permitted; no calls to external C-library functions | Hosted harness supplies testing/printing; compiler support routines require an explicit allowlist, or must be removed | Caller storage or explicit allocator callback; no direct OS service | Component object undefined-symbol allowlist, generated code, callback contract and target build |
| D: custom-entry Linux executable | Whole executable; selected types/constants and ABI declarations, no assumed hosted services | Own entry assembly, linker choices, syscall adapter; any compiler helper must be named and supplied or deliberately linked | OS services only through enumerated Linux syscalls; kernel allocation is separate from application allocation | Final ELF, program headers, entry address, relocation/dependency closure and syscall trace |
| E: bare-metal image | Whole firmware; standard type/limit headers and target declarations, own MMIO interfaces | Own reset/vector code and linker script; enumerated memory/arithmetic/ABI helpers; no hidden hosted CRT | Caller storage, fixed pools or declared arena; MMIO/interrupts/DMA, no implicit OS | Map, disassembly, undefined symbols, memory regions, stack budget and reset-to-main trace |

Profile C does not promise that the compiler emits no calls. A structure assignment may become a memory helper. Division on a target lacking suitable instructions may require an arithmetic helper. Non-native atomics may invoke a runtime. Audit the actual object and final image, including link-time optimization and libraries selected by the driver.

Caller storage still has an object-model contract. Under the C17 baseline, an aligned declared `unsigned char` array is not automatically typeless allocated storage for arbitrary typed lvalue access. A portable byte arena can expose byte operations and copy values into actual typed objects; a typed pool can use an array of its real element type. A hosted harness can supply genuine allocated storage through an explicit allocator interface. Firmware casts into a byte arena need a documented implementation contract as well as alignment, lifetime and bounds reasoning.

## Make the boundary executable

Keep `dependency-contract.md` beside the component. List allowed headers, external symbols, compiler support, startup objects, allocation sources, OS services and target assumptions. For callbacks, list whether they can allocate, block, retain pointers or reenter the component. Write down whether the promise covers only your code, the reachable call graph, or the entire executable.

Use `nm -u`, `readelf -Ws`, `readelf -d`, the link map and disassembly as complementary evidence. Undefined symbols in an object show requirements before linking; a statically resolved helper will no longer be undefined in the final executable. Search the final symbol table and inspect the relevant code as well. `ldd` belongs only to trusted executable inspection and is not a substitute for static ELF inspection. Profile D projects use `readelf` first.

An allocation-free claim needs more than finding no spelling of `malloc`. Sorting may allocate temporary storage; formatted output and locale initialization may have implementation-dependent costs; a callback may allocate. An allocation interceptor detects selected paths in the tested configuration. It does not prove that every path through an opaque library is allocation-free. Where a hard bound is required, use inspected local mechanisms with explicit storage instead.

## Distinguish six different restrictions

Removing `<stdlib.h>` removes declarations from that translation unit. It does not remove linked code. Removing `malloc` prohibits a particular allocation interface, rather than all storage acquisition. Freestanding compilation changes the implementation environment and compiler assumptions. Suppressing startup files removes the driver's usual entry objects. Suppressing default libraries removes the usual libraries. Suppressing compiler-runtime support is an additional target-dependent obligation.

Under the C17 baseline, freestanding facilities include the selected standard's required minimal headers; do not infer that a complete hosted libc exists. C23 extends freestanding library provisions. Consult the bounded standard-draft material in {ref}`guide-standards` and verify availability on the chosen compiler/library pair. A header's existence does not establish its runtime implementation.

No-library projects may use a hosted oracle that calls the equivalent mature function. The learner's component must implement the studied mechanism itself. Document the oracle's preconditions and disagreements; do not silently choose the result that makes the test pass.

## Resource ledger

For every substantial component, record persistent bytes, worst-case temporary bytes, maximum recursion/stack depth, allocation count and failure behavior. Separate measured high-water marks from analytical bounds. Include interrupts, nested contexts, alignment padding and callback costs where applicable. Profiles are constraints to investigate, not evidence that a custom replacement is safer or faster than a production library.

## Extension boundaries

Handwritten assembly in {ref}`T36` has a chosen ISA/call convention and remains subject to its surrounding C interface contract. Protocol cores in {ref}`T40`–{ref}`T42` use profile C; their hosted/firmware adapters use the separately declared profile A or E. The full Ethernet-to-HTTP family owns its driver and higher protocols while physical MAC/PHY hardware supplies the declared lower mechanisms. The hosted HTTP family owns HTTP and socket lifecycle while the OS supplies TCP/IP and the lower layers. Follow {ref}`guide-communication-stacks` for the complete ownership map.

<!-- performance-debugging guide extension -->

## Diagnostic and measurement boundaries

An instrumented investigative variant has its own dependency contract. Sanitizers, profiling hooks and logs can add calls, allocation, code, stack and timing effects. Retain the uninstrumented profile-B/C component or profile-E image audit separately; a diagnostic variant cannot silently stand in for its original dependency or resource promise.

Add named cost units, simultaneous lifetime peaks, completion/reclamation events and observer overhead to the resource ledger. Fixed link sections, whole-path conditional stack estimates, sampled high water, allocator retention, process resident bytes and cgroup accounting are distinct entries. Reuse these ledgers in {ref}`T43`–{ref}`T45`; the directory does not add another implementation budget.
