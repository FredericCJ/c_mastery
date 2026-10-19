# Current B06 document renditions

Ten newly authored documents contained seven invalid byte encodings, two valid
UTF-8 mojibake strings and one replacement character. These are teaching and
assessor prose, not deliberate malformed-input fixtures. The original files and
their execution receipts remain byte-identical. The earlier F439 public UTF-8
rendition is retained without modification.

`tools/rebind_b06_documents.py --write` produces the renditions from the fixed
original digests and exact half-open byte ranges in `encoding_map.json`. Bytes
outside those ranges, including line endings and already valid UTF-8, are copied
unchanged. Six documents have isolated Windows-1252 dash bytes. The source note
has two such bytes alongside two already valid UTF-8 dashes; decoding that whole
file as Windows-1252 would corrupt valid text and is explicitly rejected. The
compiler documents contain the exact UTF-8 encoding of a previously misdecoded
en dash. Recovery transport's replacement character is a contextual correction
to the range D1–D5, supported by the five explicitly enumerated deliverables; it
is not a claim that a lost original byte can be recovered mechanically.

`bindings.json`, the pure `author_support.attach` hook and the maintained B06
generator attach the public guide and only public renditions to the relevant
projects. The two retained DMA/lease projects are canonical Markdown inputs;
the same dedicated producer reapplies their additive frontmatter attachments.
Their sidecar criteria, historical baseline bindings and existing supplied
historical bytes remain unchanged. Private assessor renditions and the source
note never enter the public project asset lists.

The completion packet preserves the exact current nineteen B06 predecessor rows
and changes only document references, adding a declaration of the rendition
mapping. Separate document-defect identities assess the actual byte repair,
strict decoding, regeneration and export checks. No C/target campaign was rerun
or relabeled. The preserved execution inputs, source-inspection gate and existing
F439 rendition are pinned and checked. External review, learner transfer and
physical observations retain their existing limitations.

Run `tools/rebind_b06_documents.py --self-test --report <new-report.json>` for
the metadata guard. It rejects changed originals/ranges/renditions, whole-file
misdecoding, omitted defects, incorrect current bindings, assessor leakage,
changed historical states and invented execution. Its temporary export checks
use the real project asset preparation/export functions; no full build or
acceptance promotion is performed.
