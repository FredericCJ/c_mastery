# Identity assessor analysis

## T26.05.a

Equal root bits do not establish uninterrupted object identity. In the original
trace A1 and A2 share address/index0 but have different incarnation labels. The
integer CAS demonstration is defined C and proves only its numeric comparison;
it never accesses freed native storage. A generation-tagged word rejects the
particular replacement until that generation can recur. The implementation
separates validity from borrowing and preserves the originating-pool premise.
The actual CMX producer's first slot admits65535 incarnations, then remains
unavailable while the next slot can be used. This is a real imported producer,
not a claim that the new representation is its existing ABI.

## T26.05.b

Generation tags cost bits and do not provide a protected interval between
validation and access. Deferred reclamation and hazards instead prevent reuse
under their full participation/protection conditions, consuming retained
storage/records/scan work. The original hazard-pointer paper explicitly excludes
the misconception that storage protection alone eliminates all logical ABA.
A still-live node may leave and return to a list while its address and allocation
generation remain unchanged. The algorithm must protect the relevant structural
identity, perhaps with a version incremented on each structural change.

The new compact representation has8 low index bits and24 generation bits; only
indices1–16 are admitted. A maximum handle therefore remains representable in
uint32_t. Releasing the maximum retires the slot, not increments through zero.
Reinitialization with stale handles is excluded; a safe reset is a distinct
global-quiescence protocol. Ordinary array accesses require external
serialization regardless of a separate atomic root's lock-free result.

## T26.05.c

The model checks256 initial tags times256 positive advances:65280 differing
pairs and256 false equalities at advance256. It intentionally includes tag0
to enumerate the entire8-bit modular space; the pool reserves generation0 and
admits only1–255. The pool's4080 executed incarnations and final FULL therefore
test a different saturating policy. The24-bit maximum test is a boundary
projection, not execution of its whole lifetime bound268435440.

Permanent retirement exchanges stale-identity safety for finite new-allocation
capacity. A finite-state pool serving infinitely many fresh incarnations must
either repeat an encoding or eventually refuse; unrestricted indefinitely
surviving stale references make the repeated encoding ambiguous. A proof of
quiescence can permit a reset only by changing that stale-reference premise.

Native two-word atomics may call libatomic and report non-lock-free. Record
actual size/alignment, success/failure behavior and imported library identity;
do not infer its internal locking algorithm from an undefined symbol alone.
Compiler warnings and strict target linker rejection are expected evidence of
the dependency boundary. The32-bit CAS may close where a pointer+tag pair does
not. This audit establishes neither physical ISA execution nor interrupt-safe
operation, and does not upgrade a2004 architecture observation to a current
hardware specification.
