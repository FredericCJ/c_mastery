# Foundation depth exercise laboratory

Implement the selected exercise in `src/depth.c` against `include/depth.h` and
[the shared contract](CONTRACTS.md). The learner file compiles and deliberately
fails all eight semantic gates. Import accepted prerequisite artifacts through
documented adapters. Preserve prediction notes before inspecting test outcomes.

Linux/WSL learner example:

```sh
cmake -S labs/foundation_depth -B build/my-depth -DCMAKE_BUILD_TYPE=Debug
cmake --build build/my-depth
ctest --test-dir build/my-depth -R '^foundation_depth.extent$' --output-on-failure
```

The eight tests are `semantic`, `extent`, `cursor`, `codec`, `lifecycle`,
`overlap`, `handles`, and `defense`, prefixed by `foundation_depth.`. Public
semantic checking validates verdict completeness and the repaired transition;
it does not reveal the classification key. All exercises additionally require
the reasoning and transfer rubric in their canonical brief.

GCC/Clang debug, release and ASan+UBSan profiles are available through
`CMakePresets.json`. They use C17 with extensions disabled. Harness dependencies
are distinct from target-component dependencies. Strict target audits are build
artifacts only; they are not a firmware flashing recipe.

Repository maintainers run `python3 tools/run_foundation_depth.py` for the author
campaign, with explicit assessor source selection, compiled mutation controls,
learner-stub rejection, independent integer/wire oracles and a Cortex-M4 final
link audit. Exact logs and report default to `build/foundation_depth/`; historical
acceptance receipts are never overwritten. Same-author campaign success does
not establish learner acceptance or reviewer independence.
