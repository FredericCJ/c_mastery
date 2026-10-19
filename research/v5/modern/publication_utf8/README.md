# Exact publication encoding successor

The canonical T69/T70 briefs, two assessor analyses, DOD contract and measurement
analysis contained one, two or three repetitions of interpreting the UTF-8 bytes
of EN DASH (U+2013) as Windows-1252. The authority contract contained the same
single interpretation error for LATIN SMALL LETTER C WITH CEDILLA (U+00E7) in
“façade”. These are damaged teaching words and range punctuation, not deliberate
invalid-input specimens.

`tools/rebind_modern_publication.py` admits only nineteen recorded source hashes
and four literal substitutions. Its immutable correction manifest is
`research/v5/modern/history/publication_utf8_20261018/correction.json`; the same
directory preserves all prior document bytes. The manifest records every changed
line and the exact before/after code points. Every other byte, including original
line endings, is retained. All exercise metadata is unchanged except the exact
punctuation in X-T70-007-R3. That criterion retains its identifier, objective,
range endpoints, operation, rubric levels and failure obligations. The dedicated
criterion comparison admits only its complete known original and corrected
sentences, not arbitrary normalized prose.

These Markdown files are canonical authoring inputs. The tracked repository has
no active `author_dod` or `author_dispatch` script that recreates them; the narrow
repair producer is idempotent and the established `author_exercises_v5` producer
regenerates the catalog. Retained historical initial output is not rewritten.

The current publication proof is `current_binding.json`. It must include all
fifteen actual selected exercise exports, complete learner briefs (including
criteria and delayed variants), supplied contracts and fixtures, and byte hashes
of their retained workspaces. It also binds the export implementation. Exported
workspaces contain no private assessor paths. This is publication validation,
not another execution of the C mechanisms.

The predecessor execution receipts retain their original document hashes and
their observed commands/results. They are not asserted to have executed the new
whole-source identity. A consuming validator may classify only an exact approved
old document hash as a historical-execution/current-publication binding after
checking the complete immutable manifest, unchanged laboratory code, headers,
build rules and tests, and the current export proof. Every other source-pin and
execution check remains mandatory. There is no general stale-hash exception,
new learner assessment, physical observation, independent review or release
acceptance.

The first export run correctly completed all fifteen CLI commands but its
post-run verifier incorrectly expected one extra trailing newline. Its failed
producer/proof and diagnosis are retained under
`reviews/v5/expansion/modern_publication_utf8/attempt1/`. A subsequent run under
`build/` is retained as an earlier scoped observation; the final proof keeps its
actual export artifacts under the review tree so an extracted source package can
recheck them without recreating missing evidence.
