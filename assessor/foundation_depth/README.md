# Assessor-only foundation depth material

Keep this directory out of learner publication/packages. `depth.c` is a
same-author reference implementation, not evidence of independent assessment.
`semantic_answers.inc` holds the classification key; the public completeness
gate deliberately does not expose that key. The canonical exercise rubrics and
separate `analyses/` explain the technical obligations beyond finite tests.

`mutations.json` records eight mechanism-specific defective controls. CMake
builds all of them and the normal implementation with the selected compiler and
optimization/sanitizer profile. `reject_mutant.py` accepts only explicit semantic
assertion failure. Learner stubs are built separately and each of the eight
exercise gates must reject its stub. All success claims are attached to exact
sources, compilers, commands and inputs by `tools/run_foundation_depth.py`.

The strict Cortex-M4 audit deliberately excludes hosted allocation callbacks
from the reachable final image. Its compiled portable object and final image
are checked for runtime helpers and unresolved symbols, with a final link map,
symbol ownership, disassembly, ELF sections and stack-use records. The ELF is a
link-audit artifact with no reset startup or board vector table; it must not be
presented as executed or physically validated firmware.
