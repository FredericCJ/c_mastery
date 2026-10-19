# Observed P6 measurement limits

These are actual author observations from the source-bound measurement-04 binaries. All thirteen samples per cell remain in the receipt; the table shows median native nanoseconds per256-sample block for the first-block-warmed procedure. No host configuration changed.

| Profile | Blocks | Footprint bytes | Scalar | Compiler | AVX |
|---|---:|---:|---:|---:|---:|
| gcc-O3 | 1 | 4096 | 388.08 | 311.67 | 303.59 |
| gcc-O3 | 64 | 262144 | 390.05 | 296.50 | 305.31 |
| gcc-O3 | 1024 | 4194304 | 320.97 | 257.91 | 255.50 |
| gcc-O3 | 16384 | 67108864 | 343.45 | 294.74 | 296.01 |
| gcc-lto | 1 | 4096 | 404.72 | 335.22 | 302.08 |
| gcc-lto | 64 | 262144 | 393.95 | 303.59 | 319.31 |
| gcc-lto | 1024 | 4194304 | 328.03 | 259.49 | 268.71 |
| gcc-lto | 16384 | 67108864 | 348.10 | 303.47 | 302.36 |
| clang-O3 | 1 | 4096 | 245.59 | 186.27 | 165.03 |
| clang-O3 | 64 | 262144 | 268.50 | 202.53 | 178.00 |
| clang-O3 | 1024 | 4194304 | 220.56 | 152.16 | 148.06 |
| clang-O3 | 16384 | 67108864 | 287.15 | 246.25 | 246.98 |

The prediction of a monotonic cache-capacity transition was not supported across all footprints: the1024-block medians were lower than64-block medians for these profiles. At16384 blocks, medians increased relative to1024, most clearly for Clang, consistent with greater memory pressure but insufficient to isolate cache causality. Staging/validation and uncontrolled shared-host scheduling/frequency can obscure a simple capacity model. Wide retained maxima are not deleted as outliers.

Clang LTO retained computation and passed the same numerical check-only suite, but emitted packed mulpd inside the nominal scalar_kernel. That fails the declared scalar category and no timing samples are accepted for it. GCC O3/LTO and Clang O3 retain the required scalar/compiler/AVX distinctions. The failed initial linker-name, deprecated-option and category-audit receipts remain under measurement-01/02/03; only measurement-04 is the final accepted producer.

The paired scalar/backend ratios in audit.json use matching sample/workload/cache observations, not ratios of cherry-picked best samples. Synthetic90/95/100/105/110ns values independently give median100/min90/max110 and are not these native timings.
