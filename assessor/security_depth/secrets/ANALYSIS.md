# T35.09 exact assessor analysis

For .09.a the component owns only the live pool's actual four-byte array.
Temporary and malloc copies are cleared and observed before lifetime ends;
original/unmanaged copies still equal the original token. Neither zeroing
another buffer nor calling a documented primitive erases these copies.
The original dead-store function calls the external consumer then has no
further observable use of its copy; optimized selected code removes its
clear. The explicit path retains the platform call. Inspect instructions,
not merely import spelling. The selected primitive contract addresses
optimization elision, not register/scratch/opaque-provider erasure. The C23
positive library probe and actual C17/musl negative probes distinguish
frontend mode, declaration, linkage and execution. N3096 is a draft.

For .09.b, first mismatch yields early work1, last mismatch and equal yield4.
The four-iteration XOR accumulation removes only this declared count
distinction. Unequal lengths are rejected before reading input and remain
public length leakage. The eight-read table variant reveals token/candidate
indices despite identical count. It demonstrates logical address dependence,
not a cache timing attack on this CPU, F439 or another platform. All1,024
single-position substitutions establish only those finite cases. Complete
disassembly under changed optimization is retained for transfer discussion.

For .09.c, OpenSSL owns fetched/context resources internally and may retain
key-derived scratch. This adapter owns exactly two handles, one working key
copy and one staged tag. Fault1 owns no handles; fault2 owns the fetched
algorithm; faults3–7 own both. Each acquired handle is released exactly once;
fault6 still does not publish a computed tag. Fault7 is actual invalid-digest
rejection, not a fake success flag. Output/ledger admission is transactional,
and operational failure publishes only nonsensitive ledger metadata. The
input key is borrowed and untouched. A global/thread error queue is not
claimed cleared by this policy. Known-answer agreement and sanitizers do not
establish production approval, key entropy, nonce/protocol correctness or
absence of side channels.

The frozen Rph HMAC implementation supplies a separately implemented host
algorithm oracle, inspected in full and composed for all65 message lengths.
It owns inner/outer SHA states, explicit message counts and key-derived copies;
its volatile span clear still does not establish universal erasure or timing.
Python's extra oracle may share OpenSSL, so its agreement alone is not treated
as independent algorithmic corroboration.

The exact mutations fail semantic assertions: EARLY silently introduces
short-circuit work into fixed mode; CLEAR marks the pool free without wiping;
KEY skips erasure of the adapter's working copy. All are original, bounded
and use synthetic data. Public unfinished code is separately compiled and
rejected; no assessor implementation belongs in a learner package.
