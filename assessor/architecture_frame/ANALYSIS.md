# PR-T37-07 assessor analysis

The unchanged criterion tasks are implemented by a policy/ownership adapter,
not a second decoder. The frozen Xf provider is actually compiled and linked.
The target preflight calls it before entering the unchanged producer's main;
its existing reset/vector assembly, IRQ capture shim and FP callee are separate
explicit objects. No copy of a numeric trace stands in for execution.

For .09a, IRQ5 has exception index21, so its offset is84. Reset fetch is0/4
regardless of runtime VTOR; physical backing changes with the declared alias.
The basic frame occupies FE0..FFC, with PC atFF8 and LR atFF4. Its saved LR is
08000111, not the handler's FFFFFFFD. Reject unsupported policy before selecting
offsets and stage the result so malformed flags cannot corrupt a prior capture.
The stricter MSP8 admission is deliberate software policy, whereas hardware
masks only the low two initial-SP bits. The source paragraph must retain that
distinction. A vector table admitted by length is not a proof of every handler.

The a-V1 successor requires actual inspected FP/padding contracts, now supplied.
Core offsets remain0–28. Extended hardware allocation104 bytes plus optional4
restores the original SP; FP begins32 and fpscr96, with a reserved final word.
Unmaterialized FP words must never be read, even when readable backing exists.
Incorrect +18 core offsets can produce plausible values in synthetic arrays;
the actual emulator's captured PC/r0/FPCAR relationships distinguish the error.

For .09b, hardware mask is1D00F (r0–3,r12,LR,PC,xPSR). Callee mask is2DF0
(r4–8,r10,r11,SP), optionally plus200 for r9 under the selected platform rule.
Call-clobbered table is1500F. This is not a guarantee that all those values
actually change on every call. In the nested example, the inner frame's saved
FFFFFFFD belongs to outer handler A; live handler B uses FFFFFFF1, returning
to MSP/Handler. Outer A then returns Thread/PSP, recovering LR08000111.
Never interpret the inner LR as an address to ordinary executable bytes.

For b-V1, the target's group3 B capture has Handler/MSP and IPSR16 while A's
capture has Thread/PSP. Group6 turns the raw60/50 pair into equal group priorities;
B stays pending and later tail-chains against the original PSP frame. The
source uses actual implemented-bit readback, not a guessed universal value.
The normal call-graph bound includes the added preflight/wrapper activation;
do not reuse the predecessor's numeric bound unchanged. Unexpected faults stop
the experiment and are outside its normal-path stack bound.

For .09c/c-V1, compare synthetic Flash and SRAM aliases with the actual MPS2
ELF's vectors at0, MSP from the linker and Thumb reset function. F439 BOOT pin
sampling, BFB2/system-loader route and option-byte reset state are not emulator
observations. Remapping changes source bytes/alias identity and may require VTOR
relocation; it cannot be inferred from unchanged address0 alone. No actual F439
reset or mutable remap experiment is claimed. The changed task asks which
evidence must be renewed; it does not authorize flashing or BOOT pin changes.

Reject learner answers that claim eight core words preserve every register,
that readable FP allocation is initialized state, that a structural decoder is
a complete exception-return validator, or that this finite emulator campaign
establishes physical timing. The original criterion IDs, seven-day minimum and
historical accepted evidence remain unchanged.
