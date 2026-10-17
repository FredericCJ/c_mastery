# Emission, execution and expanded-subset obligations

Build emit.c with the learner core and public include directory. `emit x86
'(x+2)*3'` and `emit thumb '(x+2)*3'` produce assembly only after successful
parsing and verification. Output is a leaf `unsigned cm_expr(unsigned)` function.
The teaching notation gives literals unsigned32 type; corresponding ISO C source
uses `2u` and `3u`. Construct a constant-product counterexample showing why an
unsuffixed ISO C expression can instead overflow signed int. Do not overstate
the grammar as a compiler for arbitrary C source.

Assemble/link x86 output with an independently compiled C reference and compare
defined inputs including0,1,65535,UINT32_MAX. Generate trees independently of the
parser, evaluate their denotation in Python integers modulo2^32, and retain seed,
source, IR, machine assembly and mismatch minimization. The author experiment
runner demonstrates the procedure; its assessor-core output is assisted evidence.

For Thumb use a dedicated QEMU mps2-an386 Cortex-M4 image, an original reset
vector/linker script, semihosted bounded status and no hosted runtime. Inspect
final undefined symbols and disassembly. QEMU does not validate STM32 peripherals
or timing. Add a hostile caller that checks callee-saved registers and stack
restoration; comparing only return values is weaker ABI evidence.

Required subsequent milestones: typed local variables and lvalue/rvalue
distinction; basic blocks and conditional branches; loops with explicit control
flow; function calls, stack alignment and caller/callee saves; addressable arrays
and pointer bounds under a precisely restricted memory model. Before each,
extend grammar/type rules/IR verifier/oracle and state which ISO C behavior is
excluded or diagnosed. Require negative syntax/type cases and a changed-backend
variant. These milestones are authored transfer obligations but their reference
implementations are not supplied in this batch; the gap ledger retains them.
