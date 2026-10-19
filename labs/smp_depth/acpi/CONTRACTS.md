# T53.01: checked firmware discovery and changed byte order

This additive contract implements the concrete ordinary and delayed tasks in
`AX-PR-T53-01-T53.01.a`, `.b` and `.c`. The original PR-T53-01 prerequisites,
criterion rubric, seven-day learner delay and historical evidence remain in
force. No accepted hour, unit, project identity or old receipt is replaced.

## Inputs and required implementation

Implement `sa_parse` and `sa_discover` in the supplied `acpi.c`. Use C17, exact
`uint32_t`/`uint64_t` types, byte access and checked integer arithmetic. No ACPI
parser library, heap, unaligned typed load, packed-structure cast, physical
address dereference, hidden fallback inventory or host-endian wire cast is
permitted. Host Python, sanitizers, QEMU and linker inspection are verification
tools, not parser runtime dependencies. The public implementation is unsolved.

The base wire input is a 76-byte `APIC` table with four type-0 entries. Firmware
UIDs 10,11,12,13 correspond to APIC IDs 0,2,4,6; the third entry (UID12/APIC4) is disabled. Expected
enabled inventory is `(10,0),(11,2),(13,6)` in wire order, indexed densely 0..2.
`checks.h` supplies the bytes through explicit builders, including checksum.
Write a prediction distinguishing these three identity spaces before executing.

`sa_parse(bytes,available,capacity,profile,out)` accepts capacity 1..16 and
profiles `SA_XAPIC` or `SA_X2_RECORDS`. Inputs/output must be live and disjoint;
the input is immutable for the call. Nulls, insufficient common extent, unknown
profile, bad signature or declared length outside 44..65536/available are
`SA_INVALID`. Checksum covers exactly the declared extent; extra available bytes
are not part of the table. Invalid checksum is `SA_CHECKSUM`. The original
teaching profile requires a nonzero 4 KiB-aligned 32-bit LAPIC base, otherwise
`SA_UNSUPPORTED`; this is a profile limit, not a general ACPI rule.

Every record has at least its two-byte header and its declared length within
the remaining extent. Type0 is exactly8 bytes; type9 is exactly16 and accepted
only by the explicit extended-record profile. Type5 address override is always
unsupported. Type9 decodes full32-bit APIC ID and UID from the inspected ACPICA
layout. It does **not** enable x2APIC hardware, establish that every bit pattern
is a usable hardware ID, or authorize an IPI destination. That requires a
separate selected ISA/hardware contract. Unknown well-sized types are skipped
and counted. Reserved flag bits are ignored. Enabled entries are retained;
non-enabled OnlineCapable entries are counted separately from disabled ones.

Before publishing an enabled entry, enforce capacity and uniqueness of both
enabled UID and APIC ID, including across type0/type9 records. Capacity refusal
is `SA_FULL`; duplicate is `SA_DUPLICATE`; an otherwise valid table without an
enabled processor is `SA_EMPTY`. Capacity is checked before duplicate at the
same entry. On **every** refusal the prior output representation is unchanged.
On success all fields and retained entries are initialized in one final commit.

`sa_discover` is an original finite mapping adapter: callers supply immutable
`SaRegion` spans with exact physical bases. There are 1..16 nonempty regions,
each with a live byte object, nonoverflowing exclusive end and no physical
overlap. C object overlap with output is prohibited. It resolves complete
headers then complete tables entirely within one region; it never converts an
integer address into a C pointer. The RSDP is a separate checked span: signature,
first20 checksum, revision0 RSDT or revision2/length36/extended checksum XSDT.
Other revisions are unsupported, no automatic fallback is performed. Root and
child headers have declared lengths 36..65536 and valid checksums. Root body
length is divisible by4 or8; every referenced child is checked, and exactly one
MADT is required. Missing MADT is `SA_EMPTY`, repeated MADT is `SA_DUPLICATE`,
other malformed mapping/root/child data is `SA_INVALID`. Final topology comes
from the same `sa_parse` contract. Mapping restrictions are deliberately stricter
than arbitrary firmware support; report them instead of claiming conformance.

## Reasoning, boundaries and costs

Derive the remaining-extent loop invariant before forming each field address.
Explain why checksum validity does not imply record validity; supply a valid
checksum containing zero/one record length. Derive duplicate/capacity admission
and all-or-nothing publication. Preserve unknown fields under changed versions
only as specified, rather than interpreting uninspected new layouts.

`sa_parse` uses bounded stack storage and no allocation. Work is O(n+k*c), with
n table bytes, k enabled records and c<=16 duplicate scan capacity. Root mapping
adds O(r²) disjointness validation and O(e*r+total checked table bytes) traversal,
r<=16 regions and e root entries. A table may appear repeatedly; repeated work
is bounded by root extent but is not deduplicated by an allocation-backed map.
There is no CPU-startup, lock-free progress or physical timing guarantee here.

Test all76 truncations, declared-length mismatch, every malformed record-length
class, signature/checksum, duplicate APIC and UID, empty, capacity and online
states, mixed record widths, unaligned input, and ignored trailing bytes.
For discovery test RSDP truncations/checksums, high64-bit addresses, missing
mapping, header versus full-body mapping, overlapping regions, endpoint overflow,
mis-sized root entries and repeated MADT. `guard_pages.c` places each truncated
span immediately before an inaccessible page. Faults and sanitizer findings
are failed observations, never successful rejection evidence.

## Executable procedures and transfer

Compile `tests.c` against the completed parser with strict warnings. Its `stream`
mode consumes `profile capacity available` followed by available hexadecimal
bytes and emits status/header/counts/UID/APIC/kind fields. `oracle.py` computes
1,957 independent expected rows using Python integer arithmetic, slicing and
`struct` decoding. It also generates `oracle_cases.h` for the big-endian target.
Expected results must be computed before or independently of the C outputs.

`guest.py --lab COMPLETED_KERNEL_LAB --source COMPLETED_KERNEL_SOURCE --work NEW`
requires explicit prerequisite artifacts from PR-T48-03/PR-T53-01. It copies them
privately, checks exact instrumentation anchors, records the boot RSDP/root and
all directly referenced tables through serial, and stops at `PASS smp-madt`.
Original completed kernel files remain unchanged. The additional serial pass
visits every root child to retain bytes, even children after the first MADT;
it uses the retained `table` function and its platform range assumptions.
Run i440fx10.2 and q3510.2 with1,2,4 virtual CPUs. Record the QEMU/GRUB/compiler,
images, source identities, command, table bytes and actual topology separately.
The checked host adapter then decodes those actual regions against the oracle;
neither an inventory JSON nor the new host parser proves the retained root walk.

For the delayed endian variant, build `big_endian.c` and `big_endian.S` plus the
completed parser and generated oracle/capture headers for big-endian PPC64
ELFv2. The entry establishes the32-byte aligned initial frame and global-entry
r12 value; raw Linux write/exit are the only OS interfaces. Explicit original
`memcpy`/`memset` providers handle compiler-lowered aggregate operations. Audit
ELF byte order/ABI, zero undefined symbols and no dynamic dependency, then execute
under `qemu-ppc64`. This is an actual target translation/execution, not a host
byte-swap simulation. It is Linux-process verification of a freestanding parser,
not a bare-board runtime or physical Power system measurement.

## Deliverables, hints and assessment

Submit the exact source and independent expected ledger, root mapping and
publication arguments, ordinary/guard/oracle logs, target closure audit, all six
firmware captures, byte-order comparison and source-access limitations. Diagnose
four separate controls: ignored checksum, duplicate acceptance, truncated wide
identity, and malformed-length acceptance. A compile failure, timeout, crash or
wrong generic marker is not an intended semantic-control result. Learner stubs
must compile and fail substantive assertions; reference code is withheld.

For `.a`, level2 needs the sparse identity derivation and explicit unsupported
records; level3 needs a separately delayed wide-record implementation grounded
in actually inspected layout. For `.b`, level2 needs transactional bounds/root
mapping and all failure traces; level3 repeats the same bytes on a big-endian
target with an ABI/runtime closure argument. For `.c`, level2 distinguishes
synthetic, host, root-walk and guest capture evidence and falsifies faulty checks;
level3 changes the machine and reinspects root/table inventory and mapping
assumptions. Missing causal explanation or an overstated conformance/physical
claim fails even if output numbers match. Hints: first track remaining bytes;
then stage a local result; finally resolve numeric addresses through explicit
regions before asking for a table body. Separate assessor analysis is withheld.

## Containment, source status and limits

All fixtures are synthetic or from original disposable QEMU teaching guests,
with ordinary private images, TCG, no networking, host disk passthrough or host
security changes. No board flashing or physical hardware access is performed.
The normative-source gap is real: current detailed ACPI6.6 requests returned403.
Archived official MADT/type0 passages were actually reread; ACPICA root/type9
structures and OpenPOWER ELFv2 Rev1.5 selected pages were actually inspected.
See `research/v5/expansion/legacy_smp_acpi/resources.json` and `source_access.json`
for exact provenance. ACPICA implementation layout does not replace unread
normative root/x2 semantics. No independent human review, learner attempt,
seven-day completion, absolute mastery or physical observation is inferred.
