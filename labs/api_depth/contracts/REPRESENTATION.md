# T18.06: representation boundaries and real consumer compatibility

Required AX-PR-T18-03-T18.06.a/b/c and original delayed variants; families
A/B/C/D/F/H/I/J. Prerequisites are T03 alignment/effective type, T07 translation,
T08 ABI, T12 allocation and the contract/lifecycle package. C17 is baseline.
Implement representation.c and opaque.c against session_v1.h/session_v2.h.
The value mechanism uses no runtime library; the allocated opaque form explicitly
depends on a supplied allocator, and hosted test malloc/free are separate from
strict target dependencies. Do not cast declared byte-array storage to an
unrelated hidden structure type merely because its size/alignment seems enough.

Represent the same identity/count session three ways: exposed AcExposedV1,
allocated incomplete AcOpaque and caller-owned AcTypedV1. The typed wrapper
contains actual uint32_t words; the fields are contract-private, not protected
by C access control. Its setter/getter define the meaning of those words. The
opaque object gets storage from an allocator accepting size and alignment, owns
that allocation until destroy, and copies the allocator/context needed to release
it. The allocator/context must remain valid through destruction. Failed allocation
leaves the output pointer unchanged; NULL destruction is a no-op; a nonnull
object is destroyed exactly once. Caller-provided arbitrary pointers remain
outside the admitted contract. Getters copy values rather than leaking private
field addresses.

Change the private count field from32 to64 bits in two separately built opaque
libraries while keeping the V1 public domain and symbols unchanged. Compile
consumer_v1.c once using only an installed public header, then link that same
object against each library. Record actual private size/alignment, public symbols,
three allocation attempts and one release, including a failed attempt with an
already nonnull output sentinel. Relocate the installed prefix and relink without
private include access. The experiment tests this ABI/profile and these cases;
it does not establish compatibility for every compiler or changed calling convention.

The exposed V2 structure has a64-bit count and can change size/alignment/layout.
A new caller-storage type AcTypedV2 therefore has its own version and contract:
16 bytes aligned to8 on the selected profile. Its operations encode fields into
the first12 octets and clear four reserved octets. A separate serialized form
uses exactly12 little-endian octets: identity32 followed by count64. Numerical
loads/stores avoid structure padding and incompatible lvalue access. The cost is
fixed field extraction/insertion work, compared with direct typed field access;
measure code/stack/traffic separately before asserting a speed advantage.
Byte serialization does not require object alignment for the input octets,
but output typed objects still require their own valid alignment/lifetime.

Generate a storage-contract header from an executable sizeof/_Alignof probe,
with ABI identity0x0200. Compile assertions against the actual complete storage
type. Force a controlled32-byte alignment requirement and require translation
rejection before any access. A stale0x0100 header must fail its version assertion.
This demonstrates how generated contracts prevent a known stale build; installed
binary compatibility still requires the matching versioned symbols/ABI policy.
A truly incomplete type cannot be embedded by value using only its incomplete
declaration. Heap-free clients need an exposed/generated typed contract or an
explicit byte-serialization interface, not a guessed char array.

Inputs include count0,UINT32_MAX,UINT64_MAX and the exact serialized pattern
identity0x12345678,count0x0102030405060708. Expected octets are
78 56 34 12 08 07 06 05 04 03 02 01. Failed null calls preserve valid outputs.
Six native profiles must pass, an endian mutant and a failed-output mutant must
compile and be rejected, and both unsolved scaffolds must fail. The strict
Cortex-M4 image executes no host allocator; inspect the actual final undefined
symbols, map and disassembly. Empty runtime closure does not establish a board
measurement or performance bound.

Deliver representation/ownership and source/ABI/behavior compatibility tables,
source, external-consumer builds, generated-header provenance, intended negative
diagnostics and target closure evidence. Rubric:0 guessed layout or invalid typed
access;1 same-tree build only;2 working alternatives and exact boundary reasoning;
3 independently handles wider fields, heap-free embedding and stale headers
after the original7days. Hints: separate opaque layout from allocation policy;
state the declared type of caller storage; compile the old consumer before
rebuilding the new library. Independent review, learner performance and hardware
observations are separate from the author campaign.
