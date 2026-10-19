# ABI transfer assessor analysis

The base AArch64 ABI protects only d8-d15. Loading d8 zeros the high half, so the
17/0 result is compatible with the call rule while disproving full128-bit state
preservation. The actual guest checks all32 q registers plus FPCR/FPSR, but this
is still only the selected fixed-width floating state, not SVE/SME or an entire
asynchronous integer frame. The self-test uses private environment effects;
reusable context APIs must describe incoming state restoration separately.

The24-byte non-HFA result exceeds the selected register-result rules. AArch64's
x8 hidden pointer leaves eight explicit scalar register arguments. RV64's a0
implicit parameter consumes one argument register, so explicit h is at SP+0.
The result is36,9,8; a wrong hidden-register mutation fails in the teaching guest.
That failure is an ABI violation, not evidence of a general compiler defect.

B immediates place bits12/10:5/4:1/11 into31/30:25/11:8/7. J immediates place
20/10:1/11/19:12 into31/30:21/20/19:12. Low bit is implicit zero and the sign is
instruction bit31. Encode through uint32 masks after validating signed range.
The independent assembler words detect shared encoder/decoder misconceptions.

The distant AArch64 call produced an absolute thunk with LDR x16 from a literal
and BR x16. An initial ADRP-only expectation rejected this legal result; the
correct review classifies the emitted encoding and validates its target. The
ABI allows IP0/IP1/flags clobbering regardless of this linker's narrower choice.
No veneer execution is claimed by the link-only report.

The two-task guest installs initial return addresses and distinct SPs in owned
context records, then saves the scheduler before each restore. A task's saved PC
returns from the context-save call; the resumed path increments the same local
counter without reinitializing its stack sentinel. Sharing the two SPs destroys
the suspended sentinel and is detected. Resume admission changes ready to running
before restoration; dead-state retries return refusal with no register load.
The exact-capacity reservation moves SP down240 bytes and back while staying
inside the256-byte task extent; the241-byte request is rejected without moving
SP. Guard bytes provide a separate observation, not a substitute for the extent
proof. No preemptive scheduler or complete interrupt-save frame is claimed.
