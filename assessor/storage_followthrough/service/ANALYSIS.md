# Service boundary and compensation analysis

The packet imports the actual CVM, CST2 and Vh implementations. A C callback
test alone is insufficient for host-budget precedence, so capacity executes
OP_HOST through the retained interpreter. The observed full tree has103 keys
for the ordered workload; an attempted new fixed key needs capacity that is
unavailable. The failed transaction is aborted and every old value remains.
The host error detail is recorded alongside CVM_HOST_ERROR rather than inferred
from that coarse VM status.

The two-step workflow is a bounded compensating protocol. Its original-ID
retry argument rests on Vh committing result and receipt together. PREPARED
may survive while debit has committed; retry ID1 is harmless. DEBITED may
survive while credit has committed; retry credit ID1 is harmless. COMPENSATING
may survive while compensation has committed; retry debit ID2 is harmless.
The durable decline flag means the credit was never submitted in that branch.
An uncertain credit cannot imply decline: the supplied control reaches24 at
credit while compensation would wrongly restore the debit side to10.

Phase publication follows the corresponding effect, except the explicit
compensation decision which must precede the compensation effect. Publishing
DEBITED first can lose the debit after interruption; ignoring failed journal
publication falsely declares a usable workflow. The current implementation
poisons on both participant and journal failures and requires reopening all
images before deciding what survived.

Do not promise atomic visibility across three images or arbitrary external
effects. Another actor updating participant state violates this coordinator's
exclusive protocol, even if each individual store call is serialized. Latest
receipts are sufficient only because no unrelated actor advances those IDs.
New workflows require new image namespaces or a specified longer retention
scheme. This packet does not implement that unbounded extension.

The independent oracle computes15 exact phase/balance/7-byte intent rows.
The cut harness covers962 observed stage boundaries under two persistence
choices and74 intent-creation cuts. All image owners are reopened after an
interruption; a success assertion never reuses a poisoned handle. Costs must
include checkpoint work performed by store_begin, not just the immediate37
commit events.

Initial compilation caught an imported-header name collision: both service and
durability providers have medium.h. CMake now deliberately puts the selected
DMedium header first; actual compiler dependencies are recorded and checked.
The optimized pilot passed all five anchors and four compiled controls. Final
compiler/guest claims come only from current reports. Reused kernel evidence
retains its original source, emulator, image and exact scenario scope; no new
physical or independent-human claim is made.

The first full campaign retained a GCC sanitizer-profile compile refusal in the
key fixture: format-truncation analysis did not infer the earlier dynamic key
count bound. The fixture now constructs its six decimal digits explicitly;
no warning was disabled and no mechanism test removed. The failed attempt is
retained separately from the final source-current campaign.
