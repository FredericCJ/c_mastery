# Error-correction assessor analysis

The parity-check columns for positions1..n-1 are distinct nonzero binary indices.
Their XOR is the syndrome. A single bit produces its column; the overall bit
produces zero syndrome but odd total parity. Two different lower-position flips
have nonzero syndrome and even parity; a lower-plus-overall flip also has nonzero
syndrome/even parity. Hence the bounded decoder rejects every pair.

The kernel of the check equations is linear. No nonzero weight1/2/3 word passes
both equations, and positions1,2,3,n give a weight4 word. Thus distance4 is exact.
For two candidates agreeing within t known-bit discrepancies and s erasures,
their distance is at most2t+s, contradicting4 when the admission inequality holds.
This proof supplies uniqueness, not evidence that an arbitrary channel honors
the asserted t/s budget. Three flips1,2,3 of zero yield syndrome0 and odd parity;
the bounded decoder changes overall parity to obtain valid nonzero135 in n8.

The 16/11 code rate increases because an added check doubles the non-overall
position space; it does not increase minimum distance. The independent oracle
enumerates bit vectors satisfying parity equations and computes all distances
for the small code, instead of reproducing the syndrome branches.

For V2 use wire lane=(logical_lane+phase)%4, phase0..3. Bit-column stride remains4,
so the same burst bound follows for known phase. Wrong phase permutes entire
valid codewords: each decoder can return EC_OK while assigning payload to the
wrong logical owner. A phase token must therefore be framed/protected separately;
ECC validity does not establish frame alignment or sender authority. Keep the
admitted phase as an explicit input and reject values>=4 before output mutation.
Assess that changed representation/cost argument separately from R2 bit repair.

No cryptographic repair is claimed by this family. A trusted zero-only payload
policy rejects the supplied forgery9 but has no general authenticity guarantee.
All demonstrations are local arrays; no real communication or device is touched.
