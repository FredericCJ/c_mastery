# F439 contract encoding successor

The original `labs/firmware_lifecycle/targets/F439_UPDATE.md` is Windows1252 with
CRCRLF line endings. Strict UTF8 decoding fails at byte1369 (0x96). Reading it
with replacement decoding silently damages ranges and section symbols. The
original bytes remain at the exact path pinned by the firmware campaign and
predecessor B06 envelope; no historical input or execution receipt is rewritten.

`F439_UPDATE_UTF8.md` is the current learner/publication rendition. Its strict
UTF8 decoded string equals the original strict Windows1252 decoded string,
including original line endings. It changes no wording, requirement, voltage,
range, procedure, API, exercise identity or evidence claim. New rendition source
identity is explicitly different; it must not be passed off as the previously
executed byte identity.

The versioned six-row completion disposition replaces the exact document path
in existing current-publication artifacts. All original runtime evidence and
six-dimensional status objects are retained. The predecessor firmware report
remains the evidence for executed code. The encoding validation is solely a
metadata/text equivalence check. No library source or hardware action is needed.

Run `python tools/rebind_f439_document.py` to reproduce this rendition and its
current binding, then `python tools/rebind_f439_document.py --self-test` to check
all original identities, source hashes, code-point equality, predecessor source
closure and eight negative metadata controls. Any consumer with a source-access
pin to the old document may retain it as historical evidence and add a current
binding to the UTF8 rendition without relabeling an old runtime execution.
