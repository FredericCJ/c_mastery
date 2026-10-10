# Runtime research and scoped acceptance

## Broad search
The existing library and source registry were searched for compiler and interpreter teaching. The Knowledge library holds two Sandler PDFs; the selected copy is identified and hashed in runtime_resources.json. Existing Crafting Interpreters scanner coverage is insufficient for VM execution and collection, so additional author-hosted chapters were sought. The rendered VM/GC endpoints failed; the author repository manuscripts were read directly.

## Reflection
A full language implementation would overshadow the earlier C mechanisms. The new family should reuse checked arithmetic, bounded stacks, parsers and graph traversal. It needs an original tiny format and language, explicit exhaustion and a clear distinction between compiler evidence and VM verification. References to Lox or C do not make our subset either language. Raw manuscript inclusion markers are not inspected C listings.

## Focused search and inspection
Read the VM dispatch/value-stack narrative; call-frame local storage and return-address passages; expression parsing organization; GC roots, temporary roots, graph tracing and sweep narrative. Read Sandler variable resolution, conditional parsing and function declaration/call pages at the exact PDF/printed locators in the resource records. The author companion page distinguishes test suite, implementation and errata and supplies the official support boundary.

## Synthesis and gate
Accepted before teaching authoring: explicit caller-owned VM state; separate operand/frame stacks; staged source validation; iterative reachability and temporary roots are supported explanatory topics. CVM1 encoding, stack-height verification, fixed limits, checked int32 arithmetic, fuel, host/GC budgets and generation-tagged references are original specifications. No closure/JIT/native-FFI implementation or full C compiler is required. Tests must execute original C and reject mutations; source inspection alone earns no runtime or physical-board claim.

Passages are bounded assignments. No copyrighted book text or implementation is copied into the deliverable. Source access and gaps remain explicit.

## Final staged-gate audit

An extracted learner-runner check exposed a sequencing error in the original first gate: it reached dispatch and the hosted compiler through a combined portability test, while the codec itself called the later instruction verifier. The final API separates raw bounded container encode/decode from verified save/load. Checkpoint 01 can now pass with every verifier, dispatcher, collector and compiler stub still present; checkpoint 02 adds verified wrappers. Hand-encoded dispatch and call tests stay independent of the compiler. Compiler expression and control-flow gates are separate, arena allocation does not call the later collector, and frame-root and host-budget probes use hand-encoded code. Late portability and compiler/GC/host integration remain cumulative regressions in the full suite. These are changes to the original educational specification, not additional claims about the inspected books.

The runtime author matrix runs 14 reference tests in each of GCC debug, GCC release and Clang ASan/UBSan, plus 13 expected semantic failures in the untouched learner build. An independent reviewer supplies separate partial-implementation probes; author success alone does not demonstrate independence. The fixed ARM startup adapter interprets linker symbols as integer addresses under its documented ELF/ARM ABI, avoiding relational comparisons between separately declared C objects. Actual emulated Thumb execution remains distinct from physical board evidence.
