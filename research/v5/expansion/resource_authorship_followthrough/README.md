# Source credit metadata successor

The 36 additions in `research/v5/resource_authorship_additions.json` supply a
previously absent `authors` field on twelve source registers. The original
register files, technical inspection scopes, resource identifiers, editions,
assignability decisions and execution receipts are unchanged. No accepted
baseline resource is eligible for this operation.

`credit_inspection.json` records the actual inspected credit passages, source
identities and attribution roles. Committee preparation, named manual authors,
project contributors, editors and corporate technical documentation are
distinguished explicitly. These bounded credit inspections do not claim that
additional technical passages were read. Locally cached payloads are not copied
into publication; the public report retains their digests and locators. The
ST datasheet credit was inspected through the official web PDF after a direct
local download timed out; no local PDF digest is invented.

The existing resource discovery and identity-correction mechanism applies each
addition only after checking the exact original register and row, the evidence
record, the corrected row and the absence of an existing authors field. A
different source, stale capture record, accepted resource or any non-author
change fails. `tools/check_resource_authorship_v5.py` exercises these boundaries
in temporary sandboxes and checks the real 36-row correction set. Ordinary
resource regeneration then consumes these canonical corrections; no separate
reapplication step or historical inspection rewrite is required.
