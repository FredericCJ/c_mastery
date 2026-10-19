# Separate assessor analysis for the six original I2C criterion pairs

## T38.04.a

Nominal Rmin=(3300−400)*1000/3000Ω; Rmax=300*10^7/(8473*100)Ω.
Inward rounding yields967..3540; at400pF the upper integer is885. The low endpoint
must round up, not truncate. The reference's64-step unsigned division is a
restoring algorithm with a carry indicating the65th remainder bit. Before each
shift r<d; carry or r≥d requires subtracting d. Unsigned wrap implements the low
64bits of that valid subtraction. Under admitted products the quotient plus a
possible ceiling increment cannot overflow. Maximum charging numerator is
18,000,000,000,000,000,000, below UINT64_MAX; largest output upper bound is
1,180,219,520Ω for nominal inputs, below UINT32_MAX. Verify the exact integer
bound independently rather than accepting an approximate decimal claim.

For V1, minimum actual resistor is Rnom*(1−tol), maximum Rnom*(1+tol).
The low-state available sink current is IOL−Isource. For charging, current over
30–70% is at least300*Vmin_mV/Rohm−Isink_uA. Required average charge current is
0.4*Vmin_mV*C_pF/t_ns µA. Enforcing the lower current throughout the interval
gives a sufficient upper bound. This deliberately conservative construction is
not the exact leakage exponential solution. Rounding and corner admission must
be assessed independently of matching the reference representation. Reference
proofs do not turn synthetic leakage assumptions into selected-device ratings.

## T38.04.b

Lexicographically smaller MSB-first binary stream wins while larger streams
release on their first difference. Equal streams both survive. Compare every
trace mask as well as final wire value; a buggy model that never removes losers
can still produce some correct AND bytes. Clockmax-low/min-high describes the
initial shared interval only. The implementation does not claim a losing
controller clocks beyond the byte boundary. START/STOP markers are rejected
before output publication; NXP leaves the mixed phase combinations undefined
within the protocol, so manufacturing a winner is the tested misconception.

## T38.04.c

The original synthetic capture with decodedA0,07,A1,5A and310ns rise fails the
300ns timing bound.295ns plus10ns uncertainty also fails;605ns high minus10ns
uncertainty fails Fast-mode600ns high minimum. A Standard-mode800ns rise with
adequate low/high intervals passes only the small modeled timing predicate.
The physical gate still requires raw verified measurements. Nominal2.2k/100pF
rise is186.406ns;4.7k/100pF398.231ns;2.2k/400pF745.624ns. Changing mode may
admit the last timing bound but requires device, frequency and other timing
conditions to be revisited. The decoded-only mutant intentionally suppresses
these predicates and must be rejected. No current campaign contains board data.

## T38.05.a

The actual Pi driver writesA0,07,A1. Address/data acknowledgments belong to the
target receiver; final NACK belongs to the controller receiver. For one byte,
ACK is clear before ADDR clearing and STOP is requested before receiving data.
For two bytes, POS is set and ACK clear before ADDR clearing; RXNE alone must
not prematurely read both bytes. BTF permits the prescribed STOP/two-read closing.
Caller bytes remain unchanged until successful terminal cleanup. Address NACK
on A0 or A1 is not the controller's normal final NACK. The fixture checks accesses
while the unchanged F439 code checks actual register offsets and critical scope.

## T38.05.b

Address-write/address-read rejection return5, data rejection6, ARLO7, timeout4
under the retained enum. ARLO has priority over coincident AF and does not issue
STOP against the winning controller. Other failures may need pending STOP
cleanup before returning the original cause. Old-token events must return3
without callback reads, phase advance or publication. The derived local mutant
removes only the step token comparison from a copied provider source. Its
bounded counterexample reports `late_publication 0 195 0 3`: token1 publishes
token2'sC3, clears phase and performs three callbacks. Correct behavior is
`late_publication 3 17 9 0`, followed by the legitimate token2 completion. This
is actual publication corruption within an original teaching program, not a
memory-safety or code-execution claim. The imported frozen provider is unchanged.

## T38.05.c

The adapter rejects absent exclusive ownership or other controllers before any
write. Idle lines cause no operation, satisfying the original PR38 fixture.
For stuck SDA, actual Pi recovery is delegated; after1..9 release attempts it
can finish, after the ninth unsuccessful attempt it returns PI_IO, and held SCL
times out without inventing a rising edge. Caller policy names reset_required;
there is no such Pi enum and no automatic reset action. Test wraparound with a
monotonic modular sequence and admitted half-range deadline.

V1 counterexample: recovery observes both lines high under an untrue lease
assertion, another controller emits START, and the recovery controller pulls SCL
low as its next pulse. The waveform may now truncate or extend the other transfer;
neither one idle sample nor the software budget grants ownership. Correct remedy
is an externally established lease covering the full operation, or refuse
recovery and escalate through a separately authorized topology policy. No finite
GPIO state machine guarantees safe recovery under unrestricted competing masters.

## Assessment and evidence limits

Award level2 only for the objective-specific derivation, actual producer edge,
failure ledger, independent expected-result comparison and first-divergence
diagnosis. Level3 requires the exact delayed V1 after seven days, unaided changed
artifacts and an explicit remaining-evidence argument. Supplying these reference
implementations is author work, not an independent learner assessment.

The source-bound campaign requires six native profiles, four actual M4 CPU
profiles, all finite oracles, meaningful compiled mutants, copied learner-stub
rejection and target link/stack checks. The conservative stack sum covers every
retained nonrecursive C frame plus32bytes; IRQs are masked, so nesting and
physical watermarks are excluded. Native timing, bus speed, physical waveform,
board route and independently reviewed acceptance are not measured here.
