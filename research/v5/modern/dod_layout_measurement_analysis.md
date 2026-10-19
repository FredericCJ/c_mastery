# Layout conversion and changed access patterns

The current `dod_measurements.json` receipt contains672 samples: GCC/Clang,
three populations, warm/eviction-conditioned runs, eight operations and seven
trials. Row and column updates retain the same checked signed arithmetic and
unchanged-output rejection contract. The conversion paths now execute the
actual field-list-generated `d_rows_to_arrays` / `d_arrays_to_rows` functions,
with disjoint spans, explicit capacity and poisoned destinations. Every field
is checked after timing, and Python independently computes workload checksums.

At1048576 rows in the warm condition, GCC O3 median row/column update durations
were1457820/801792ns. Converting rows to columns cost875020ns and converting back
966994ns. A simple median-only estimate needs two hot updates to recover one-way
conversion and three to recover both directions. The trial ranges are wide;
combining pessimistic observed endpoints would need more iterations and is not
a confidence interval. Clang's corresponding medians1450276/803390ns and
1193632/842121ns produce different crossover estimates. A workload that performs
only one update cannot ignore conversion while claiming the split layout wins.
Lifetime, memory footprint, ownership and whether a row-form consumer is needed
also change the decision.

The same whole-record indexed workload reverses the median ranking: GCC
4990773ns rows versus8158834ns columns, and Clang5046190ns versus9131057ns.
All representations return the same checksum. This is an observed counterexample
to a universal SoA speed claim, not a model for every record size or access order.

Actual conversion disassembly differs. GCC's row-to-column body contains packed
loads and shuffle operations with guarded/remainder paths. Clang's main copy
loop in this build uses scalar loads/stores, unrolled for two rows. Its early
packed instructions combine pointer checks, so searching for an XMM register
alone would falsely establish a vectorized data loop. Compiler reports and
full disassembly are preserved with commands and hashes. No source-level SIMD
claim or another architecture's instruction support is inferred.

The64MiB stride touches condition eviction but cannot prove cold caches. No
affinity/frequency control or hardware counter was used. A second ABI/processor
and stricter cache control remain changed-environment observations. The exact
named host conversion/traffic/crossover investigation is implemented and run;
external review and independent learner defense remain separate requirements.
