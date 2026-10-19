# Query index author validation

Run `python3 tools/run_corpus_query.py`, then
`python3 tools/check_corpus_query.py --self-test --output
reviews/v5/expansion/corpus_query/guard.json`, and
`python3 tools/run_corpus_query_export.py` from the checkout under Ubuntu.
The JSON reports retain exact commands, tool versions, source hashes, raw
output, exit statuses and target artifacts. `tools/close_corpus_query.py`
checks those records before constructing the objective-specific dispositions.

The 2026-10-18 campaign passed 75 commands. GCC15.2 and Clang21.1.8 each ran
debug, release and ASan/UBSan builds, all three anchors, six compiled semantic
mutants and a compiling learner-scaffold rejection. The independent raw-row
oracle used seed731207, 728 datasets, 8736 queries and 113568 observations
per compiler. It scans membership, consecutive tokens, distinct occurrence
pairs and scalar three-valued truth; it does not rebuild the optimized index
as its expected-result algorithm. Zone expectations include old and new extrema
for conservative updates. Finite generated cases complement the invariants
and worked counterexamples; they do not establish universal correctness.

Four GCC/Clang Cortex-M4 builds at -O0/-O3 retain the actual component object,
original helper object, linked image, linker map and two static stack reports.
The explicit original helper provider is `assessor/execution/support.c`.
Component imports are checked against the documented original-helper allowlist;
helper objects and final images have no unresolved symbols or linked libraries.
Two host helper checks each executed1040 cases. These are cross-build and
runtime-closure observations, with no Cortex-M execution or board claim.

The GCC host measurement records sizeof(QiIndex)=1064, sizeof(QiRow)=14,
sizeof(QiPosting)=2 and sizeof(QiZone)=6 for that ABI. The same10000 inclusive
range queries visited10000 groups/40000 rows in sorted order and60625 groups/
242500 rows after permutation. Recorded ticks4321 and4523 are single bounded
observations, not stable latency estimates. Candidate pruning work and final
predicate work remain separately counted. Layout, query distribution, compiler
and mutation policy must accompany any comparison.

The claim guard rejected16 deliberate report mutations. Actual canonical CLI
exports passed33 commands for the three exercises: initialization, compiling
unsolved-scaffold rejection, both explicit assessor-provider root forms and
missing-provider refusal. No assessor source was copied. The export pins the
three executed catalog contracts and all CLI Python dependencies; it does not
claim that unrelated catalog entries were executed.

The null-logic, distinct-occurrence proximity and conservative-zone delayed
variants have working private references and independent checks. Independent
technical review, learner performance and the seven-day delayed attempt remain
pending. Concurrent readers, persistence and dynamically unbounded indexes need
additional contracts and are outside this bounded in-memory package.
