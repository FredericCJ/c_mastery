# Exact original allocation, copy and hot-unplug changes

Import completed unchanged B4, B5 and B045 providers explicitly. All models are
serialized C17 operations on live initialized disjoint objects. They do not
construct native addresses, access real page tables, free native pointers or
offline host CPUs. Zero-initialize controls. Reinitialization requires a new
owner lifetime with no surviving tickets. Public stubs remain incomplete.

## X-T45-001 V1/V2: incarnation and owner are separate

SxLedger composes actual b4_memory for every accounting transition, retaining
four slots and live/peak semantics. IDs are nonzero. Allocations and successful
nonzero resizes get a new slot generation1..3; failed resize preserves the
generation, prior size, totals and ownership and returns the same alias. Retire
an exhausted free slot rather than recycle it. A successful resize at generation3
is refused before the modeled allocator operation. Release remains legal.
An alias contains slot, generation and identity; lookup/release/resize must also
match the current owner0/1. Integer aliases never establish native-pointer validity.

Each allocation records an immutable allocator identity0/1 and current owner0/1.
Transfer changes owner and attribution but leaves global live/peak bytes,
allocator identity and generation unchanged. Old owner authority immediately
fails even if it retained an identical alias. Release requires the allocation's
original allocator. Copying an alias creates no new allocation or destruction
obligation. One successful release ends ownership; duplicates fail unchanged.
Per-owner totals sum to global live. The actual moved native resize/initialized
tail experiment remains the separate frozen MwOwner provider, not this integer
ledger. Transfer does not authorize one allocator to free another's object.

## X-T48-001 V1: staged copy and mapping revision

SxMap owns four original B5 pages and nonwrapping mapping versions0..3. remap
replaces one page's permissions and initialized bytes and increments its version;
version3 cannot remap again. A caller changes mappings only through this API.
copy snapshots the pages and versions, runs actual b5_copy_to_user into staging,
then invokes an optional bounded callback. It validates every touched version
again and commits only the requested bytes if they still match. Callback effects
are retained even on refusal; failure guarantees no copy bytes were committed,
not rollback of a remapping operation. Untouched page changes survive success.

Calls are externally serialized, including recheck plus commit. The callback is
an explicit interleaving seam, not a claim that ordinary C loads solve concurrent
page-table races. It may call remap but cannot recursively copy, corrupt metadata,
retain borrowed storage or destroy owner/source. Nonwrapping versions prevent
ABA; a real kernel must implement the declared serialization/pinning/fault domain.

## X-T48-001 V2: partial copy is an explicit result

partial imports the actual B5 copy for one byte at a time and stops on the first
permission fault. A successful API call publishes {copied,status}, even when the
copy status is CONFLICT or RANGE. Invalid required pointers return INVALID with
the report unchanged. Range rejection reports0/RANGE with no destination writes.
On permission fault exactly copied prefix bytes are committed; the suffix is
unchanged. The caller reports partial effects, repairs permissions if authorized,
and resumes at address+copied/input+copied for count-copied. Replaying the whole
operation is not automatically safe for a side-effecting destination.

## X-T53-001 V2: an explicit quiescence handshake

SxHotplug composes actual SShootdown. CPUs0..7 have online/draining/quiescent bits
and nonreused incarnation1..3. Begin admits only online, nondraining targets and
captures each incarnation. offline_request closes that CPU to future admission
but leaves it online and pending in any active request. Timeout is not completion.
offline_quiescent is a trusted supervisor event asserting all execution using
the old address space has stopped and cannot resume, with required invalidation
or loss of that translation context already complete. Only this exact-incarnation
event can fulfill its pending acknowledgment and remove the CPU from online.
The event's external truth is a premise, not established by the callee's name.

Ordinary ACK requires captured incarnation and request generation. Rejoin is
permitted only with no active request, advances incarnation and starts after a
fresh local context/invalidation setup supplied by the supervisor. Saturation
refuses rejoin. Old offline/ACK events cannot clear a successor incarnation.
No target-mask deletion occurs merely on request or suspicion. Frozen StWorld
batching and actual QEMU guest invalidation remain separate evidence; StWorld's
fixed-online contract is not silently changed into native CPU hotplug.
