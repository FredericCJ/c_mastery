# Bounded storage and lane measurements

The source-bound `reviews/v5/expansion/modern_completion/storage_lane_measurements.json`
records1512 timing observations:216 cells, seven rotated trials each, GCC and
Clang at O0/O2/O3. Builds, compiler versions, object sizes, full disassemblies,
observed values and log hashes are in that receipt. The initial O3-only study
was extended because T70.03.d explicitly requires multiple optimization levels.
No source-level optimization flags were weakened after diagnostic failures.

The retained-heap oscillation performs32+C explicit payload-byte writes and one
allocation/release for C cycles across the16-byte threshold. Compacting on
every cycle performs16+33C writes and C allocations/releases. For C=64 the
observations are96 versus2128 writes and1 versus64 allocations. These are
source-statement observations, obtained from separately generated instrumented
copies whose exact transformations and hashes are recorded. They exclude
metadata stores, compiler-generated struct copies and physical memory traffic.
All timing binaries omit the observer. Both policies have the same final bytes
and cleanup contract, but retain different capacity between operations.

The actual byte codec writes8N payload bytes (encode then copy) while the typed
union writes4N on construction; N=1..8. The exact T66 allocator observes no spill
for N<=4 and one allocation per owning representation for N>4. The borrowed
slice comparison owns no capture. Runtime call timings exclude construction;
they therefore do not stand in for allocation/encoding latency. Observed sizes
are DSmall64, fixed-inline24, typed capture56 bytes, with pointer/owner alignment8
on both available compiler configurations. This is one host ABI, not a portable
layout guarantee. The capacity0 quota model separately proves that preserving
old contents during growth can require old+new live bytes, even when the final
allocation alone would fit.

The lane study evaluates the same clamping result for unsigned8, unsigned16 and
signed16 lanes over1024 and65536 packed words. Independently computed Python
checksums agree with every timed invocation; the C scalar baseline also checks
the kernel. O3 signed16 median times for65536 words were373918ns scalar versus
94391ns packed for GCC, and297724ns versus109658ns for Clang. At O0 they were
1617704/192831ns and1646901/198783ns respectively. These observations describe
these binaries and workload, not a universal crossover or speed requirement.

Inspection of both O2/O3 signed-lane disassemblies found integer masks,
addition, XOR, shifts, subtraction and OR with no branch or call in the kernel.
O0 retains stack temporaries. The primitive uses unsigned bit-lane arithmetic
and decodes no out-of-range signed C value. The absence of a data-dependent
source branch does not establish constant execution time, side-channel
resistance, ISA availability elsewhere or surrounding-loop vectorization.
Compiler versions and full output are recorded so those claims can be revisited.

Host scheduling, caches and frequency were not controlled. Trial minima/maxima
are retained rather than discarded; instruction counters and another ABI remain
unobserved. The study closes the named owner-footprint/copy/hysteresis and
multiple-optimization lane investigations at this host scope. Learners must
still produce their own cost dossier and delayed transfer evidence.
