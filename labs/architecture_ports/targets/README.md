# Two explicit target ports

PR-T58-01–08 require actual reset/trap/link experiments, not only the hosted
register model. The assessor assembly is a voluntary context save/restore leaf,
not a full exception frame: RV64 saves ra/sp/s0–s11; AArch64 saves x19–x30 and SP.
The caller provides live aligned storage of112bytes/RV or112bytes/AA64. AArch64
SIMD callee-saved registers and FP status are excluded: the caller must compile
with a no-FP contract or extend the frame. No thread may resume an expired stack.

Learner tasks: implement context.S from these offsets and ABI passages, cross
assemble; write a linker script and reset vector for a named emulator machine;
initialize stack/data/bss; implement a hostile callee that fills preserved regs,
switches contexts and checks them; inject one synchronous trap; validate the
saved PC/status and return. Record readelf/nm/disassembly/map and every helper.
Compare privileged architecture rules to the ABI; a saved ABI context does not
constitute a privilege transition. Supervisor page permissions require a real
page-table walker/TLB experiment with an inspected target manual. Missing target
manuals/emulators remain required gaps. No deployed device or host settings.
