# Public reproduction on native Ubuntu

Install the build, diagnostic and target tools with `bash tools/bootstrap_ubuntu.sh --install`.
This explicitly uses sudo for Ubuntu packages and installs the locked Python
dependencies in `.venv`. The script leaves PMU permissions and other host policies
unchanged. `python3 -m cmastery doctor` reports discovery; actual execution probes
in the campaigns establish which capabilities work.

Use the public profile from a source checkout or its public source review archive:

```sh
.venv/bin/python tools/check_public_profiles.py
.venv/bin/python tools/build_v5.py --profile public
.venv/bin/python tools/validate_expansion_v5.py --profile public
.venv/bin/python tools/check_candidate_exports_v5.py --profile public \
  --work-directory build/public-candidates --report reviews/public-candidate.json
```

Choose a new report filename for each candidate run. The public build validates
the real exercise contracts and references, canonical JSON and Python syntax,
the active contextual route, preserved historical identities, independent fixture
oracles, and actual foundation and algorithm implementations under GCC and Clang.
It executes the PMU protocol and deadline pilot, including sanitizer and mutation
checks, without requiring PMU access: the PMU protocol, loss-accounting, ring,
parser, clock and publication fault-injection tests run in debug, optimized and
ASan/UBSan builds; the ring consumer and its real drainer thread also run under
ThreadSanitizer, which must catch two deliberately weakened memory orderings;
and the native collector's argument checks run against an ASan/UBSan build.
The deadline collection tests run in debug, optimized and ASan/UBSan builds.
The PMU and deadline ASan/UBSan builds use `-fno-sanitize-recover=all`, so a
sanitizer finding fails the run instead of only printing a report; the CMake
`instrumented` profile (`labs/algorithms/cmake/Profiles.cmake`) does not set it,
so there a UBSan finding is reported without necessarily failing the test. Public validation only
configures the build profiles: it verifies the compile and link sanitizer
policy of the semantic, instrumented (ASan/UBSan), thread (ThreadSanitizer) and
performance CMake profiles but does not build or run them. Building every
profile, including the ThreadSanitizer profile, and running its tests is done by
`tools/check_native_build_profiles.py` without `--configure-only`. It also
requires semantic rejection of unimplemented learner scaffolds. Its `build/public/validation.json` records exact
commands, output hashes, tool identities and the unchanged source identity.
Each invocation keeps immutable logs and campaign receipts in a fresh
`build/public/runs/` directory, so build and validation can run consecutively.
The active learner resolver produces `build/public/html/index.html`; subject
catalog pages remain references. The candidate check rebuilds an extracted
source archive, compares source identity and generated HTML, then actually
initializes a project and study profile from the separate learner archive.

Public source review archives contain author implementations for reproducible
checks. Learner archives exclude assessor content, author scripts and research
analysis; the assessor archive is separate. No private captures or retained
author binaries are supplied to the extracted build. Review archives are not
accepted releases and are never promoted by these commands.

The default `author-evidence` profile keeps the existing strict author and
publication requirements. `tools/accept_v5.py` and `tools/package_v5.py` retain
their full acceptance gates; a public result cannot satisfy those stronger
claims. Missing private source inspection, target experiments, hardware
measurements and independent human assessment remain separately tracked.

Campaign code may resolve a tool through `tools/native_tools.py`: an explicit
argument takes precedence over `CMASTERY_TOOL_<NAME>`, then PATH. For example,
`CMASTERY_TOOL_LD_LLD=/usr/bin/ld.lld-21` selects an installed linker. A bad
override fails visibly. New runs record the actual tool path, version and hash;
historical receipts still identify their original tools and machines.
