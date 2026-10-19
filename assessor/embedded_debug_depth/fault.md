# Private fault/stack technical analysis

The selected core publishes only one record. Its release store follows all plain
record writes; acquire readers cannot see a partially constructed first record.
Later attempts affect only the atomic diagnostic. The nested test must observe
BUSY during capture, then the original status after publication and after another
attempt. This is a bounded protocol, not a claim that C alone defines an NMI ABI.
The target separately audits lock-free unsigned atomics and actual generated code.

Status validity and pointer readability are independent. A set BFARVALID bit makes
the captured numeric BFAR meaningful under the acquisition premise; it never
authorizes dereferencing that address. A stacking failure invalidates frame
interpretation even when eight words happen to be accessible. The imported
xf_frame decoder is deliberately used with FPU capability false, so extended
frames are marked incomplete rather than read using guessed offsets. The nested
FP assessment concerns maximum simultaneous allocation, not an unsupported
extended-frame capture implementation.

The original fixture gives three true flags. Clearing only BFARVALID must leave
the stale raw register unpublished. The target instead causes an actual MPU
data violation with CFSR82; its unchanged owned guard and successful adjacent
write distinguish protection from a generic fault. The handler may continue
only from the exact labelled STR to its exact labelled next instruction after
capture. This is intentionally narrow and must not become an arbitrary fault
resume recipe. The offline parser uses actual executable ELF bytes and a digest
bound by acquisition, not the target's synthetic token or source filename.

The restricted T1 decoder yields base+4*imm5 and uses captured r0..r3. A write
also requires its value register. It does not infer an LDR result that never
completed. Combined/imprecise status cannot establish unique causal PC. Source
identity mismatch precedes symbol interpretation, even when symbols look useful.

Lockup simulation preserves the architectural distinction: precise BusFault at
negative priority depends on BFHFNMIGN, imprecise BusFault remains pending, MPU
data depends on HFNMIENA and selected UsageFault locks up. A richer logging
routine can make the first record unavailable by refaulting; fixed capture only
reduces dependencies under explicit readable-storage premises. Never equate a
synchronous test hook with actual architectural lockup execution.

Stack arithmetic:256-96=160 observed free;32+64+32+32=160 conditional live
allowance;160+2*(104+4)=376 with two extended padded frames. These are different
quantities. A live160-byte C object containing64 fill-valued bytes followed by96
changed bytes is a concrete undercount, not a corrupted pointer test. Canary
sampling misses changes elsewhere; a32-byte MPU guard catches only accesses
within its range. Recursion and unknown callback stack consumption invalidate
the proposed whole-path bound until separately constrained.

Meaningful controls remove address validity, image equality, imprecise caution,
FP accounting or first-record immutability; each must compile and fail the
corresponding assertion. Target controls omit the MPU or corrupt the captured
base, and the public stubs fail semantically. Sanitizer errors, timeouts and
unrelated QEMU failures do not substitute. Author-team code inspection and finite
tests do not establish independent human review, learner assessment or physical
fault behavior.
