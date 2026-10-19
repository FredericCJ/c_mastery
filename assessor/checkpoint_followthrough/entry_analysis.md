# P2 entry-input integration assessor analysis

The existing CPW writer, original entry instructions and byte-helper implementations are imported unchanged. This packet adds a consumer whose changed argc, payload and environment inputs actually alter observations. The old campaign's second argument was ignored; it remains useful historical fixed/PIE evidence but does not satisfy that new input observation by itself.

## T09-01-b

The relocatable aggregate object can carry an unresolved memcpy reference and has no loadable process image. The final static image resolves that reference to the declared byte-loop provider. Inspect the image object's nonempty .data/.bss and final PT_LOAD file/memory difference. The loader establishes zero storage beyond file bytes; the linker stores the initialized sentinel in the file. Startup C must not claim to have initialized either before its own first read. e_entry must reside in an executable load range, but that alone says nothing about calling convention or relocation support.

Delayed PIE: derive runtime-minus-linked offsets independently from cpw_data and AT_ENTRY/_start. The two biases agree. Fixed images give zero; selected relocation-free PIE children give a page-aligned nonzero bias. A stored absolute pointer adds a dynamic relocation even when every symbol resolves; the checker must reject that image before execution because no relocation stage is supplied. A changed ASLR placement is not an ABI change or a claim about entropy.

## T10-01-c

The process entry starts with argc at the original stack pointer, not a C return address. _start preserves that pointer in the first-argument register, establishes caller alignment and calls cpw_image; the call itself pushes the return address. Argument pointers begin one word later, followed by the argc-indexed null and environment pointers. Auxiliary entries follow the environment null. A loop bound does not make a forged pointer valid: the inspected Linux-created stack is the trusted source premise.

The changed-input packet uses actual zero/one/two payload arguments, empty/maximal payload, a changed second payload and CE_TEST values. Successful output is the concatenation of admitted payload strings; these are actual semantic inputs. Invalid argc/length/environment refuses before stdout. Kernel auxv entry and data-symbol observations bind the runtime image. Do not call the selected ABI draft an ISO rule or claim this single x86-64 packet validates another architecture.

## T11-01-c

The final link explicitly excludes startup/default libraries and stack-protector/unwind dependencies. The separate65536-byte aggregate forces an actual helper requirement on the selected compiler modes; link without its provider must fail. The retained runtime helper performs byte operations without recursively importing memcpy. Its object and the CPW portable core are identified by exact source hashes and final maps. The audit also rejects a fully resolved malloc symbol, demonstrating why an empty undefined-symbol list is insufficient.

Changing GCC/Clang and O0/O3 rebuilds every object and the final image. Preserve header/driver/ABI/tool identities; changing compiler can change which helper is emitted, so prior absence of a helper is not a universal proof. The actual helper-copy check covers each of65536 bytes in this finite image. It does not establish every compiler lowering or target. Independent learner delay/reasoning and external review remain separate from author execution.
