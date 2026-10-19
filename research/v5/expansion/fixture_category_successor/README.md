# Fixture-category inspection successor

The embedded and networking fixture directories now contain two distinct kinds
of public input: finite synthetic milestone input/expected-output pairs, and
blank forms for future physical captures. The previous fixture entry points
attempted to interpret every JSON file as a milestone. The current entry points
validate each category against its actual project and assessment bindings.
Unknown files, malformed forms, missing ownership and populated forms remain
failures. A valid blank form is not a physical observation.

`history/verify_embedded_fixtures.py` and
`history/verify_networking_fixtures.py` preserve the exact predecessor bytes.
Their SHA256 identities are also fixed in `tools/fixture_inspection_successor.py`.
The accepted baseline assessment records and their inspection passages retain
these predecessor identities. This successor does not claim a new historical
inspection, rewrite an execution result, or qualify the entire current files as
previously inspected.

The qualification is limited to the original expressly named synthetic model
branches. It checks the exact old whole-file hash, current whole-file hash, and
every original top-level source span other than `main`: imports, constants,
model and validation functions, and entry-point guard remain identical and in
order. Added definitions cannot shadow original names or their existing global
and builtin lookups; new module-level executable statements, decorators,
annotations and executable default arguments are rejected.
The changed orchestration is separately authored and executed. This is a
bounded structural check, not a general equivalence proof for arbitrary Python.

`proof.json` binds a current validation receipt. That receipt pins exactly the
two verifiers, the networking template helper, both category regression suites,
and the qualification helper. Its separate input map binds the actual fixture
files, project contracts, consumed assessment sidecars, module catalogs and
presence-checked public protocols/checkers. Four exact command invocations retain
their raw output and process status: both verifiers and both category regression
suites. The qualifier checks these identities, output schemas, mandatory case
counts and zero physical claims. A failed, absent, stale or malformed receipt
does not qualify an inspection. Platform-specific skipped controls remain
visible in raw unittest output; no skipped control is relabeled executed.

To reproduce the current proof, retain the immutable history files and run:

```text
python -B -X utf8 tools/fixture_inspection_successor.py --refresh --report-dir reviews/v5/expansion/<fresh-run-directory>
python -B -X utf8 tools/check_fixture_inspection_successor.py
python -B -X utf8 tools/fixture_inspection_successor.py
```

The refresh refuses an existing report directory, retains the prior active proof
there, runs the four checks, and publishes only after source/input stability and
all qualification checks pass. It does not update baseline assessment records or
historical execution receipts. If an inspected model, import or constant must
change, this mechanism rejects the change even if current hashes are repinned;
a separate substantive source inspection and validation is then required.

The initial failure, preimages and current raw commands are retained under
`reviews/v5/expansion/integrated/final_author_run/fixture_fullbuild_fix/`.
Physical acquisition, learner implementation, delayed assessment and external
review remain separate and are not supplied by this proof.
