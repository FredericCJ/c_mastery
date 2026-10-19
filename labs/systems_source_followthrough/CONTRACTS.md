# SE-SYS-HEX-01: record semantics versus loader policy

This required source companion belongs to T09.10.c and T09.12.a–c and extends
the original sparse-image investigation. Families A, B, C, D, F and I are
assessed. Source register: `research/v5/expansion/systems_source_followthrough/resources.json`,
R-SYS-FOLLOW-HEX88, all format sections, printed pages4–11. The retained
GDB debug-companion reading remains R-LELF-DEBUGLINK in the ELF source register.

## Prerequisites

Bring the PR-T09-05 sparse-byte/image manifest and its independent checksum
argument; know checked extents, unsigned arithmetic, byte decoding and the
difference between an entry address and a load destination. Required prior
knowledge also includes T03 byte views and T04 integer bounds. A current
passing reference run does not supply those learner artifacts.

## Environment

Hosted ISO C17; GCC/Clang, eight-bit bytes and available `uint32_t`. The parser
uses only supplied character bytes, integer operations and caller storage.
The host tests may use stdio/string routines; the implementation must not call
an existing HEX decoder. Inputs and the live, properly aligned output object
are disjoint. No pointer is formed from a decoded address. No entry is called,
file deployed, device flashed, or external service contacted.

## Problem

Implement `sf_hex` in `hex.c` according to `hex.h`. Accept at most4096 input
characters and16 records; return at most64 addressed bytes in record order.
Recognize hexadecimal digits in either case, a leading colon, even digit
pairs, record length and checksum; admit LF or CRLF line endings, including
no final newline. A CR without LF, blank line, data after EOF or missing EOF
fails. Sum decoded record bytes including checksum modulo256; exclude colon
and line endings. Each successful call replaces `*out` once. Every rejection
preserves every byte of its pre-call representation.

Three explicit input profiles keep ambiguous mixed conventions out of the
contract: plain accepts00/01 and refuses an offset crossing65536; segment
accepts00/01/02/03; linear accepts00/01/03/04/05. The active base starts at0.
An extended-address record contains exactly two bytes at offset0. An entry
record contains exactly four bytes at offset0, is retained as `entry_kind`
and a big-endian32-bit `entry` value, and never changes a data address. A
second entry record fails, even if identical. EOF has zero length/offset.
Unsupported type/profile combinations fail. Duplicate data addresses fail,
even when bytes agree. These rejection policies are explicit teaching-loader
constraints, not claims that the historical format forbids every such file.

Use32-bit modular arithmetic for linear addresses and16-bit offset wrapping
before adding the segment base. Distinguish an entry's two register fields
from a computed execution pointer. Input pointer/output pointer NULL, invalid
mode or input length above4096 returns `SF_INVALID`. Within a record, framing
and length are checked before checksum, then record semantics and capacity.
Return the first encountered rejection. `SF_CAPACITY` is reserved for record
or output-byte bounds. A short odd pair is `SF_FORMAT`.

The delayed admission adapter `sf_window` borrows a live decoded image. It
checks every populated address against `[begin,end)` without changing the
image. Empty images pass a valid window; NULL, count above64, `begin>=end` or
`end>2^32` is `SF_INVALID`. An out-of-window address is `SF_RANGE`. Entry
metadata is deliberately not admitted for execution by this data-only API.

## Necessity

The original small decoder deliberately accepted a narrow generated subset.
That cannot establish the semantics of all six record forms. A plausible
incorrect explanation says every extended address is simply a high word,
and another treats the start record as the next load address. Both fail on
the supplied changed-address cases. Checksum acceptance does not authorize
an image and cannot establish compatibility with a processor or programmer.

## Reasoning

Prove that every byte access lies in the input and that the decoded length
agrees with the remaining record. Bound staging storage and explain why a
late failure cannot publish an earlier record. State the active-base,
unique-address and single-entry invariants. Derive wrap results without
executing signed overflow. With N<=64 output bytes and L input characters,
the simple duplicate scan costs O(L+N²); fixed arrays make memory bounded.
Explain which different representation would change this cost.

## Deliverables

- D1: sealed record-by-record predictions for both wrap fixtures and all
  rejection classes, with the two false explanations refuted.
- D2: the completed C17 parser and a representation/invariant/ownership
  account, including failure precedence and all output-preservation checks.
- D3: exact commands, inputs and observed outputs from `tests.c` and a
  separate integer-address oracle; add one original discriminator.
- D4: after at least7 days, independently solve `SE-SYS-HEX-01-DELAYED`:
  change from linear to segment addressing at a boundary, retain CS/IP
  metadata separately from data destinations, and compare a wrapping source
  format with a nonwrapping programmer admission window. Derive the window
  check before implementation and reject the same checksum-valid input when
  its decoded destinations lie outside that window.

## Verification

Compile the learner file with `tests.c`, then use `driver.c` for an independent
oracle. The author campaign is `tools/run_systems_source_followthrough.py`.
Every ordinary case, capacity bound, invalid shape, checksum, duplicate,
entry interpretation and transactional rejection must pass. Retain compiler
versions and source identity; ASan/UBSan passing is bounded execution evidence.
The supplied script tests160 address/count configurations plus entry and
capacity cases per profile and requires distinct checksum, segment-carry,
entry-as-data and first-address-only admission mutants and an unsolved stub
to fail semantically.

## Rubric

- R1/T09.12.a: level0 misdecodes or overruns;1 reproduces the sparse example;
  2 derives every supported address form and refusal policy;3 transfers the
  delayed window rule without confusing address span with populated bytes.
- R2/T09.12.c: level0 admits corrupt records;1 quotes a checksum formula;
  2 derives and tests it independently and preserves output on late failure;
  3 constructs a checksum-valid unauthorized/unsupported image counterexample.
- R3/T09.10.c/T09.12.b: level0 equates entry, data and debug identities;
  1 reports file length;2 separates entry metadata, sparse bytes, address span
  and retained debug companion identity;3 defends the changed target's policy
  and names which earlier evidence no longer applies.

Level3 is an independently reviewed delayed attempt, not a reference-test flag.
Assessor technical analysis is separate at
`assessor/systems_source_followthrough/ANALYSIS.md`.

## Hints

1. Validate one encoded record before changing base or collecting bytes.
2. Compute an address for each byte; a record may cross a boundary.
3. Stage the entire output. Put entry metadata in a different field from the
   active base, and make the profile restriction explicit.

## Variants

In addition to the required delayed window variant, change the output to sorted
disjoint runs and account for adjacency, overlap and bounded insertion cost.
Change transport to fragments ending in the middle of a pair, retaining
ownership of incomplete input without publishing output. These are genuine
representation/state changes, not a demand to vary constants mechanically.

## Security

The input is synthetic local text and the output is inert address/value data.
No program-memory write or entry transfer is implemented. Adversarial records
may violate parser bounds or admission assumptions; a rejected record or
sanitizer finding is not demonstrated code execution. Any later device loader
requires its own authority, memory map, erase/commit, signature/freshness and
recovery contract; a valid HEX checksum provides none of those properties.

## Validation

Authored and actually executed reference evidence is recorded separately in
the source-bound campaign. Learner implementation, external assessment and
physical programmer behavior remain unassessed. The full historical format
reading does not turn this deliberately bounded decoder into a universal tool.
