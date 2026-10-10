# Reading spine and source use

Read small assigned passages in support of a project. The full bibliography records editions, access, inspection method, limits and teaching backlinks. The local Knowledge library holds actual documents; the SWE graph is a discovery index. A graph node or contents page is not evidence that a chapter was read.

## Priorities

| Layer | Selected spine | How to use it |
|---|---|---|
| Language and interfaces | {ref}`R-modern-c-3`, {ref}`R-effective-c-2` | Selected object/interface/allocation explanations, checked against draft and implementation boundaries |
| Representation and design | {ref}`R-hanson-cii`, {ref}`R-parnas-modules-1972` | Understand ownership and information hiding; critique old assumptions before implementation |
| Algorithms and numerics | {ref}`R-clrs-4`, {ref}`R-HIGHAM-ASNA-2`, {ref}`R-GOLDBERG-FP` | Derive invariants/error reasoning; C representation and overflow remain your responsibility |
| Systems | {ref}`R-SYS-CSAPP-GLOBAL-3E`, {ref}`R-SYS-TLPI-2010` | Bridge source to linking and Linux behavior; use the verified edition's locators |
| Firmware and testing | {ref}`R-WHITE-EMBEDDED-2`, {ref}`R-GRENNING-EMBEDDED-TDD` | Selected explanatory passages, subordinate to Arm/ST device and board contracts |
| Primary contracts | WG14, compiler/linker manuals, POSIX/Linux, Arm/ST | Resolve semantic, ABI, OS and register questions for the declared environment |
| Foundational arguments | Hoare, Dijkstra, Parnas, Wilson, Lamport, Boehm–Adve, Liu–Layland, Roofline | State the original assumptions, then relate them to the present project |

The resource directory contains actual record IDs; where one work has separately inspected records, use the specific locator record assigned by the module. A library book is not required cover to cover. Paid sources have focused public primary-document complements and original teaching notes. If access is unavailable, use the contract source and module explanation, and record the substitution; do not obtain an unauthorized copy.

## Reading notebook

For each assignment, keep four short entries: the mechanism it explains, a prediction for your project, a version/assumption that limits it, and a test or argument that could refute your interpretation. Compare the reading with a source excerpt or generated artifact when the module asks for implementation literacy.

Source reading recurs: allocator policy, libc sorting/startup, Linux ELF loading, compiler runtime behavior, a tagged RTOS port and startup templates. Inspect manageable named functions or subsections. Reading code does not grant permission to transplant it: record its license before reuse, and write the educational implementation independently.

Historical material is valuable because it exposes decisions. K&R's allocator assumes pointer-order behavior beyond portable ISO C; Hanson contains deliberately problematic exercises; old UNIX/ABI texts describe their own versions. Explain those limits in the notebook and modernize the contract rather than treating age as either authority or disqualification.

## Evidence limits

The extension uses focused algorithm derivations from CLRS/Knuth, representation and interface work from Hanson, and architecture explanations from Hennessy/Patterson, with original Dijkstra publications and pinned Intel/Arm ABI/ISA documents. Communication teaching distinguishes conceptual X.200 layering, exact MCU/PHY/transceiver manuals, and normative Internet RFC passages. Module reading cards select the actual inspected edition and locator; public workshop summaries and original educational contracts carry their narrower labels. The complete search/reflection/synthesis record is published with the enhancement research.

Required readings are validated against exact registry locators. New chapters need further inspection. A bounded selection supports its stated purpose, not an entire domain. No assigned reading implies complete standard conformance, successful hardware behavior or a completed formal proof. The public research reports retain inaccessible alternatives and unresolved evidence needs.

<!-- performance-debugging guide extension -->

## Performance and diagnosis spine

The new tracks assign small actual inspected passages: {ref}`R-clrs-4` for formal and amortized bounds; {ref}`R-ENH-HENNESSY-PATTERSON-6` for quantitative models; {ref}`R-PERF-VITTER-2001` for external-memory units; and {ref}`R-PERF-NIST-SP500-235` for static control-flow complexity. These measures answer different questions.

For diagnosis, {ref}`R-PD-DDMIN-ISSTA2000` supplies a reduction argument, {ref}`R-PD-VIDOVIC-ARM-2026` supplies a held example to critique, and exact GCC/GDB/Arm/ST locators supply the tool and target contracts. For memory experiments, the held {ref}`R-effective-c-2`, historical {ref}`R-MEM-ASAN-PAPER-2012` and versioned sanitizer/Memcheck/linker/source passages distinguish mechanisms from detection limits. A historical overhead figure or development-manual version is not a prediction for an installed setup.

The public performance/debugging research records broad search, reflection, focused search and source synthesis separately. Module reading cards retain exact locators and time allocations. Official documents and original teaching support substitutions where a held commercial source is unavailable; record the substitution and any narrower claim.
