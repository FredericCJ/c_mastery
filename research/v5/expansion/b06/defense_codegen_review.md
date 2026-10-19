# DT3 generated-code author review

Reviewed the actual four `dt_equal` disassemblies retained in
`defense_transfer_validation.json` after the passing campaign, not just source
or compiler optimization reports. This is author review, not external review.

GCC O3 with vectorization enabled and disabled emits the same scalar loop.
Entry branches check the two256-byte limits, output pointer, nonempty input
pointers, equal lengths and zero length. The loop's only conditional jump compares
the advancing index with the public length. Bytes feed xor/or into the accumulator;
the final comparison uses SETE rather than a content-dependent branch. Stores to
the steps output follow the public index. No content-dependent address or branch
was observed in either of these concrete functions.

Clang O3 with vectorization enabled emits a length-selected packed path. Its
additional branches test a minimum length, parity/tail counts and overlap between
steps storage and each input extent. Those runtime alias checks use public
addresses and lengths. The packed path combines bytes using xor, unpack and OR;
its backedge compares the loop index with the length rounded to four. The scalar
tail and final SETE use the accumulated difference without a content branch.
Clang with vectorization disabled emits a two-byte scalar unroll, selected by
public count/parity; its loop and tail branches also depend on those counts.

These observations cover the exact functions, compiler flags and hashes in the
receipt. They establish a bounded generated-code dataflow review, not universal
constant-time execution: cache state, speculation, target instruction latency,
address confidentiality and future compiler changes are distinct assumptions.
The declared public-length policy permits immediate unequal-length rejection.
The288 timing samples include first/middle/last mismatch and equality against an
external OpenSSL comparison, but timing alone is not the acceptance argument.
