# Process and thread acquisition failures

Read CONTRACTS.md and the parent AX study sheet. Implement the two functions in
src/posix_lifecycle.c using the supplied header. The default implementation is
an intentional failing scaffold. Source inspection of the listed primary
contracts precedes coding; predict failure outcomes before reading fixture
expectations.

From the repository root, initialize an isolated workspace with
`python -m cmastery init PR-T23-01 ../launch-work --family posix_lifecycle_depth`
or `python -m cmastery init PR-T25-01 ../pool-work --family posix_lifecycle_depth`.
The explicit family selects this additive scaffold while preserving the older
project's laboratory route. The exported `ASSESSMENT.md` supplies the full leaf
criteria. Use `ctest --test-dir build -R posix_lifecycle_depth.launch` for the
launch anchor or `-R posix_lifecycle_depth.pool` for the pool anchor; completing
one anchor does not complete the other project's assessment.

From this directory run `cmake -S . -B build`, `cmake --build build`, then
`ctest --test-dir build --output-on-failure`. Tests create only original local
helpers and threads. The C17 program requires Linux/POSIX; unsupported systems
are unavailable, not passing. Record compiler, flags, libc/kernel and actual
commands. The parent study sheets contain per-objective rubrics and delayed
variants. No physical or external reviewer evidence follows from these tests.
