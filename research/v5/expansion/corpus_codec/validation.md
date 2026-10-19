# Bounded compression author evidence

The source-bound producer is `python3 tools/run_corpus_codec.py`; the claim
checker is `python3 tools/check_corpus_codec.py --self-test --output
reviews/v5/expansion/corpus_codec/guard.json`. The separate actual export is
`python3 tools/run_corpus_codec_export.py`. Run from the repository root under
Ubuntu. The current JSON reports retain full commands, exit codes, source hashes,
raw logs, target artifacts and bounded oracle results.

On 2026-10-18, the final campaign executed61 commands: GCC15.2 and Clang21.1.8
debug/release/ASan+UBSan profiles, four compiling semantic mutants per profile,
the compiling learner rejection, four Cortex-M4 object/link/stack audits, and
two measured host workloads plus independently executed driver oracles. The
15 claim-removal controls passed. The actual isolated export executed11 commands:
canonical CLI initialization, compiling semantic rejection, both explicit
private-provider root forms, and rejection of a missing provider. No assessor
file was copied into the learner workspace. These are author observations.

The failed initial target run is retained under
`reviews/v5/expansion/corpus_codec/failed_implicit_helper/`. Arm GCC's lowering of
aggregate zero initialization imported memset despite the freestanding options.
The corrected reference initializes the relevant bounded arrays explicitly;
the rerun's actual objects and linked images have no unresolved symbols and no
linked libraries. This is a property of these recorded builds, not a guarantee
that every compiler emits the same calls. No Cortex-M execution is claimed.

For the recorded Arm GCC -O0 build, the deepest compression call chain is
cc_compress(32) + compress_using(824) + cc_hencode(1160) + cc_lengths(448) +
copy_bytes(32) =2496 bytes from the actual static frame reports. The optimal
parser's alternative chain is32+824+1088+32=1976, so it does not dominate that
build's Huffman path. Expansion's deepest chain is824+632+168=1624 bytes.
These sums cover this function call graph, not an interrupt stack, startup,
caller storage or a whole application. Optimization changes inlining and tail
calls; inspect the corresponding disassembly before reusing this sum.

The GCC host observation emitted28-byte equal-data frames,31-byte periodic
frames,191-byte ramp frames and200-byte permuted frames for128 input bytes.
The optimal token parser and greedy parser had equal final sizes on these four
particular workloads, while their CPU ticks differed. The retained independent
small-message search supplies an actual greedy-token counterexample; it does
not make optimal token count equivalent to optimal entropy-coded frame size.
Ticks include function calls and are one bounded host observation per pair,
not a statistical performance claim or physical target latency.

Independent technical review, learner performance and the seven-day delayed
attempt remain pending. The completed delayed reference and oracle do not
stand in for those other actors. Finite validation is not absolute mastery.
