# Operator-stack author validation

Run `wsl.exe -d Ubuntu --exec python3 tools/run_corpus_shunting.py` after changes.
The source-bound `reviews/v5/expansion/corpus_shunting/current_validation.json`
records75 actual commands, six GCC15/Clang21 C17 debug/release/ASan+UBSan profiles,
three substantive anchors, seven compiled semantic defect controls and three
compiling scaffold rejections. The independent recursive-grammar/exact-integer
oracle executes4,512 cases per profile (seed150018;4,490 valid and22 malformed),
also comparing the actual prior Pratt tree fields/evaluator and streaming output.
Two additional compiler-built drivers repeat the oracle outside CTest. These are
author observations, not independent learner attempts or proof for all strings.

Four Cortex-M4 GCC/Clang O0/O3 core object and linked-image audits retain24
object/image/map/stack artifacts with hashes. Only permitted generated memory
helpers resolve to original `assessor/execution/support.c`; the linked images
have no unresolved imports or library archives. Actual helper checks execute
1,040 conditions under each native compiler. These audits cover the parser core;
they do not execute a board or certify the entire lexer/evaluator target pipeline.

Both native compilers measure whole and transactional token-at-a-time paths over
1/11/21/31/41/51/61-token addition expressions,10,000 parses each (14 rows per
compiler). The observed host state is2,872 bytes. For61 tokens, both paths perform
30 operator pushes and30 reductions, while streaming stages620,000 complete
states over the10,000 runs. Recorded GCC host CPU ticks were2,757 whole and26,790
streaming; these are one bounded run, not a portable ratio or latency guarantee.
The Arm GCC O0 frame records include sy_parse2,160 and nested sy_begin2,136 bytes:
their simultaneous4,296 bytes precede any caller/interrupt/context allowance.
Clang O3 instead reports sy_parse2,136, consume40 and reduce32-byte frames.
Do not substitute host sizeof or one function frame for a whole-call stack bound.

`tools/check_corpus_shunting.py --self-test --output
reviews/v5/expansion/corpus_shunting/guard.json` passed16 evidence-corruption
controls: missing profile/anchor/mutant/mapping/stub/target/artifact/oracle/cost,
stale/failed state, forbidden helper/library, invented target run, physical result
or learner mastery. `tools/check_corpus_shunting_exports.py --report
reviews/v5/expansion/corpus_shunting/export_validation.json` performs real isolated
CLI initialization of this exercise and its actual X-T15-012 prerequisite, both
explicit assessor-root forms, semantic learner refusal and missing-prerequisite
rejection. Its receipt, rather than this paragraph, establishes execution status.

The original source is Dijkstra's inspected1961 ALGOL Bulletin Supplement10;
registered page scope and exact PDF bytes remain in resources.json. The small
grammar and transactional API are original extensions. External technical review,
physical target observations and an independent delayed attempt after seven days
are pending separately. Existing accepted parser identities/hours are preserved.
