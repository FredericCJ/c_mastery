# T65.07 timer investigation: assessor only

The window invariant is stronger than individual elapsed<half: each resident
age remains below half. Since each original delay is also below half, age-delay
lies in(-half,half), so the modular comparison has the same sign as the integer
timeline. Among due entries, greatest age-delay is earliest absolute due time.
Equal values retain insertion order; unrestricted pairwise serial sorting lacks
this common-window premise. A due but uncollected item still ages and can block
admission. Rejecting advance is an honest failure of this service model, not a
clock correction; require external recovery if real elapsed time has escaped it.

The independent oracle sorts original integer delays and computes eligibility
using admitted elapsed. It never calls or restates the C queue comparator. The
same low pair now0/deadline1 denotes absolute1 overdue at16 or absolute17 future.
The absolute queue resolves this only by adding information and a finite lifetime
contract. Its display_bits cannot alter eligibility. At UINT64_MAX, cleanup/pop
must remain possible; signed conversions and overflow-before-check fail review.

R1: retain the exact three policies, admissible intermediates and serial cycles.
R2: examine transaction preflight, unchanged output on AGAIN and stable ties.
R3: require actual ER13/14 and six new controls, independent pair/triple/timeline
results and staged versus raw ckd_add. N3096 is a draft, not published C23.
R4: use an unfamiliar display width, derive rate/residence lifetime bounds and
provide an external elapsed authority contract. A constant substitution or a
larger wrapping type alone does not earn transfer. No independent learner or
physical result is claimed by these author fixtures.
