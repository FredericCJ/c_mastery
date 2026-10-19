# CE-M6 independent byte and verifier entrances

Required T60.05.a/b/c continuation under `curriculum/v5/contracts/b06.md` and the
compiler extension contract. Bring CEIR1 layout, the learner verifier, parser and
transactional error tests. The original combined text/byte harness is insufficient
evidence of a separately seeded byte campaign. Keep these experiments distinct.

`ir_fuzz.c` calls only the byte decoder, then verifies accepted IR, requires exact
canonical re-encoding and checks failed decoding/output publication. It never
calls the text parser. `verifier_fuzz.c` constructs bounded one-function typed
control-flow candidates directly, bypassing both parser and byte decoder; its
three-byte local-type envelope is followed by LE32 opcode/argument pairs.
Accepted candidates execute with fuel1024. Failure must preserve the language
result. The verifier must reject invalid stack joins and targets before running.

D1/D2 derive why each entrance reaches different defenses; D3 retains separate
seeds/logs, Clang ASan/UBSan and20,000 inputs per entrance; D4 includes truncation
and explicit invalid-CFG regression evidence from the original suite. D5 states
max input4252 bytes and fixed512-instruction/32-stack bounds; D6 names omissions:
one-function raw candidates do not cover every acyclic call graph, and no finite
fuzz campaign proves verifier soundness. Actual author command is
`python3 tools/run_b06_ir_fuzz.py`, report `ir_fuzz_validation.json`.

For `.a` assess format/verifier distinction; `.b` independent seeded coverage and
publication checks; `.c` require CE-M6-DELAYED's versioned critical/noncritical
section policy with new canonicality argument after seven days. Levels0–3 and
hints remain in the original shared/extension contracts. Hint: a corpus containing
only source text rarely enters a valid binary graph. Original generated programs
and bounded local processes are the only executable targets; a fuzz finding does
not establish arbitrary code execution. Author runs are separate from learner
assessment and source-independent verification.
