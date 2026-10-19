# PR-T37-08 assessor analysis

Original time is N*CPI/f seconds. Multiply by10^9 for nanoseconds; applying
the accelerated fraction gives T*((1-p)+p/s)+C. For original T900000,p3/5,s3,
the saved work is360000ns. Fixed communication below/equal/above that threshold
makes total work faster/equal/slower. At500000ns the total1040000 gives45/52
speedup. Infinite local speed still leaves360000+C, so its500000ns limit45/43
does not promise a speedup at finite local speed. For p=1,C=0 the limit is
unbounded. Nonrepresentable uint64 intermediate fractions are rejected even
when a larger arithmetic system could represent the mathematical result.

The actual C predecessor includes a full sortedness scan. Comparing it with
the raw assembly leaf would unfairly remove n−1 admission comparisons. The
matching wrapper calls the already implemented `ab_checked` with trusted0.
Both wrappers then operate on the same live, immutable data and publish the
same first occurrence. The exact C bodies and accounting macro are extracted
with source/body hashes; the original handwritten assembly file is compiled
unchanged. Generated assembly from the C compiler is not called a separately
authored assembly implementation. Actual5125 linear-oracle checks cover the
changed high-bit and empty cases; a signed-branch mutant is rejected.

The64 queries at size16 give257 C search comparisons and960 sortedness
comparisons; size1024 gives641 and65472. These are distinct from the actual
native instruction mix or elapsed duration. An inlined admission loop may have
very different cost from a call plus assembly leaf at small sizes. Do not
force this experiment into a desired speed ranking or Amdahl timing equality.

Synthetic medians100/95 and range maximum120 must remain labeled synthetic.
The native harness retains31 paired blocks×two kernels×two sizes×two preparation
policies. Each timing interval is a batch of actual external calls, with
correctness checksum outside the interval, plus an independently checked
untimed query pass. Separate translation units and actual disassembly establish
retained work for these builds; this is not an abstract prohibition against
all future link-time elimination. The no-work control must fail its calibration
checksum before any successful measurement claim.

The displacement intervention touches8MiB before a batch, not each query.
Most of a batch is reused data. Cache topology, shared tenants, scheduler,
frequency and virtualization remain uncontrolled. Endpoint CPU equality is
only an observation. Use paired differences to reduce approximately shared
within-block disturbance, and label any causal interpretation conditional.
Do not remove high samples or subtract enough timer overhead to create negative
durations. Calibration target misses are retained and reported, not filtered.

For changed drift, common drift100i cancels in paired C−assembly differences,
all equal5. Scheduling all C first and assembly later gives unpaired mean
difference−3095, a spurious reversal. An added ten-unit within-block order effect
gives alternating−5/15 differences and median−5 for31 blocks: pairing itself is
not immunity to differential drift. This original deterministic model is not
a measurement of the actual machine's drift.

Keep original criterion/delay identities. The submitted code and author traces
support level2 readiness only; level3 requires independently observed transfer.
No physical MCU timing, energy or universal performance guarantee is supplied.
