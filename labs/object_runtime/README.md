# Managed-runtime object models

Read CONTRACTS.md and X-T74-001 through X-T74-011. Edit src; public headers,
tests and CMake are trusted harness inputs. Learner stubs compile and fail the
semantic anchors. Assessor source is separate and selected only by explicit
CMASTERY_ASSESSOR_ROOT pointing to assessor or assessor/object_runtime.

Use CMake and CTest in an initialized workspace. The repository author runner
`python tools/run_object_runtime.py --report build/object_runtime/report.json`
runs six compiler profiles,22 compiled semantic mutants,11 stub controls,
independent models and a separate Cortex-M4 link audit. It is not a learner or
physical acceptance receipt. ADAPTERS.md defines actual learner integration;
the public bounded kernels do not automatically import assessor libraries.
