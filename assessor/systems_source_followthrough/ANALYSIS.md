# Source companion assessor analysis

Keep this file and `hex.c` out of learner distributions. The implementation is
an original staged decoder, not copied vendor code. All decoded bytes, active
base and entry data are temporary until EOF and full input validation succeed.
An earlier record must not leak into the caller on any later rejection.

For segment base0x1234, the base address is0x12340. Two bytes beginning at
offset0xFFFF land at0x2233F and0x12340. For linear high word0xFFFF, they land at
0xFFFFFFFF and0. These distinguish adding a carry to the segment base from
wrapping the offset first. The source's segment equation does not require
silently truncating the final sum to20 bits. A teaching programmer may refuse
that resulting address; refusal is its own policy, not a source-format rewrite.
The encoded03 entry0x12345678 retains CS=0x1234 and IP=0x5678. No parser step
executes that entry or changes a data destination.

For R1, require both the causal arithmetic and an independent discriminator:
a learner who makes the examples pass by special cases has not established
the invariant. For R2, include a valid first record followed by a checksum
error, duplicate or capacity exhaustion and compare all output bytes with a
pre-call copy. Add a checksum-valid unwanted address: a checksum is error
detection, not trusted authorization. For R3, a start record contributes
metadata but no data byte; omitted gaps differ from explicit fill; an ELF
debug companion remains tied to the correct build/address mapping.

The sealed delayed window example uses the segment wrap above. Admit only
the half-open device window[0x12340,0x20000): the second address is inside and
the first is outside, so whole-image admission must fail without a partial
device write. The linear version with base0x0001 instead places both bytes at
0x1FFFF and0x20000; under the same window its second byte fails. A caller that
checks only the record's first address misses one of these cases. Demand an
independent all-destinations proof, not just replay of this pair.

The source inspections also refine systems claims. musl1.2.5 has separate
static TLS initialization and dynamic-loader stages; later dynamic TLS creates
new per-thread vectors before publication and uses a thread-list lock and
memory barrier. glibc2.43's segment mapper owns file mappings and partial-page
zero fill; relocation and protection restoration are separate operations.
The inspected lookup body delegates lower-level symbol matching, so a
function-name chain is not a proof of all version/search/lifetime rules.
No source-to-observed-binary reproduction or entire-runtime correctness is
claimed by these readings.
