# Algorithm and architecture companion laboratory

Implement small, bounded mechanisms in [src/algo.c](src/algo.c), then original
assembly in [src/x86_64.S](src/x86_64.S) and [src/thumb.S](src/thumb.S). The default
source compiles and fails semantic tests. Each mechanism belongs to the existing
T14–T17, T36, T37 or T43 project named in [family.json](family.json). A mapped test
is partial evidence; the project's written proof, debugging, measurement and
transfer requirements remain in force.

Copy this directory to your learner workspace. It contains no assessor source
and never discovers a sibling solution directory. On Linux or WSL, with CMake
and a C17 compiler:

```sh
cmake -S . -B build
cmake --build build -j
ctest --test-dir build -R '^algorithms.ring$' --output-on-failure
```

Implement one mechanism, make its independent tests pass, explain a failing
variation, then proceed. Consult [SPEC.md](SPEC.md) before coding and reveal
[HINTS.md](HINTS.md) one stage at a time. The harness's tests and complete boundary
adapter are visible. An assessor may explicitly configure
`-DCMASTERY_ASSESSOR_ROOT=/absolute/path/to/assessor/algorithms`; this is solution
access and is not the default learner route.

Use the project's exact test names from `family.json`. Arena and pool, sum and
Q7 arithmetic, union-find and graph paths, and ALU and cache each have separate
gates. The two assembly tracks provide `x86_64_lower` / `thumb_lower`, then
`*_insert`, then `*_sift`; the latter two retain lower-bound ABI checks as an
earlier prerequisite but do not call each other's kernel. For example,
`python3 thumb/run.py --case lower` works while insertion and sift remain stubs.
The unqualified assembly, lifetime, numeric and models tests remain combined
regressions. A full-family run includes them and all registered assessor fault
variants; passing a narrow gate does not accept later mechanisms.

`-DCMASTERY_SANITIZERS=ON` instruments hosted C using ASan/UBSan. It does not
instrument assembly or prove the absence of defects. Native assembly tests run
only on Linux SysV x86-64. Thumb tests require `arm-none-eabi-gcc`,
`arm-none-eabi-objdump`, Python 3 and `qemu-system-arm`. CMake registers those tests
only when the tools exist; missing registration is **unavailable**, not a pass.

```sh
python3 thumb/run.py --report /path/to/evidence/thumb.json
```

The Thumb runner records QEMU Cortex-M4 behavior and optionally emits the actual
disassembly beside the report. Compare source labels, instruction bytes, scaled
addresses and branches with your invariant ledger. QEMU time is not MCU cycle
time. No third instruction set, physical STM32 validation, DMA timing or bus
electrical measurement is provided here.

The assessor configuration adds nine deliberately wrong builds: three C
mechanisms, three native x86 kernels, and three Thumb kernels. Their tests must
reject the wrong semantics. Native CTest `WILL_FAIL` cases count a nonzero
semantic exit as the expected result; build failure is never their oracle.
The Thumb driver additionally requires the guest's explicit failure marker,
so a missing compiler, build error, timeout or guest crash cannot pass a mutant.

Run `build/algorithms_measure > observations.csv` for nine hosted observations
of a fixed lower-bound workload. The CSV includes validation and counter
overhead; comparison counts cover only search decisions. Preserve compiler,
flags, OS, machine, placement and load separately. There are no timing pass
thresholds and no claim that elapsed nanoseconds are CPU cycles. Use this same
kernel in T43's PMU, cgroup, compiler and MCU experiments, retaining each method's
unavailable outcome and calibration requirements.

Source reading comes from existing bounded inspections of CLRS fourth edition
(`R-clrs-4`, held document `99f5aeb0-74bc-5125-afce-59aa34ee286e`), Hanson first
edition (`R-hanson-cii`, held document `3029e68b-4b9f-565b-98d8-c26c0a61c311`),
[Morin's linear probing chapter](https://opendatastructures.org/versions/edition-0.1f/ods-cpp/5_2_Linear_Probing.html),
[AMD64 ABI draft 0.99.6](https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf),
[pinned AAPCS32 2025Q1](https://github.com/ARM-software/abi-aa/blob/c51addc3dc03e73a016a1e4edf25440bcac76431/aapcs32/aapcs32.rst),
and [Cortex-M4 DUI0553B](https://documentation-service.arm.com/static/5f2ac76d60a93e65927bbdc5).
The curriculum registry identifies exact inspected locators and edition hashes;
the older AMD64 draft is deliberately identified. This laboratory adds original
implementations and executable evidence, not a new claim of full-book reading.
