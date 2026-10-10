# Machine/image contract

The target is little-endian x86-64 System V/Linux, not portable ISO C. `mi_mix` uses unsigned modulo-2^64 arithmetic. `mi_asm_sum` may read exactly count aligned uint32_t elements; zero count allows null. The assembly caller poisons callee-saved registers.

`mi_move` permits overlap within the supplied live byte storage; it uses uintptr_t ordering as an explicit target assumption. Null is permitted only for zero extent. `mi_compare` compares unsigned bytes and returns a sign, not a required difference. `mi_divide` defines division by zero as zero and must not emit external runtime helpers. `mi_write_all` returns the complete positive count or the raw negative error; on a late error earlier bytes may have escaped, so retrying the whole call is forbidden.

Artifact subprocesses deliberately compile with -ffreestanding/-fno-builtin and link with -nostdlib when a strict image is claimed. Sanitized hosted tests do not establish the strict binary claim: the separate uninstrumented object/image audit does. The audit must reject unavailable or malformed reports rather than accept an empty substitute. No dynamic loader, libc, libgcc, compiler-rt or libatomic dependency is allowed in the entry, syscall and local arithmetic images.

Design checkpoint: choose loop direction for overlap and bitwise division invariants before coding. Describe rejected alternatives, a discriminating counterexample and a reversal condition. Keep the start.S capture of the original stack separate from the aligned call stack.
