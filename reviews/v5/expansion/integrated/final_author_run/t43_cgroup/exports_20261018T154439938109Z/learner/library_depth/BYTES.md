# T12.03–04: byte ownership and publication contracts

Required PR-T12-02 original six criteria and six changed conditions, families
A/B/C/D/F/I/J. Use C17, the parent's profile B permitted primitives and the actual
prior learner cursor/formatter artifacts. Complete bytes.c using original loops;
libc is permitted only in the separate comparison notebook, never as a shortcut
for the copy/move mechanism. Read R-LLIB-N1570-BYTES plus the format source record.
The source is the explicitly identified C11 draft; published ISO edition gaps
remain. Required prerequisites are live object extents, integer range checks,
pointer provenance, object representation, ownership and failure-atomic state.

`lb_move(base,size,destination,source,count)` operates within one live byte array.
Validate offsets against size, then count against each remaining extent using
subtraction, before pointer arithmetic. Move backwards when destination is later
than source, otherwise forwards. Derive why unread source survives each step.
The explicit example0,1,2,3,4 moved0..4 to1..5 yields0,0,1,2,3. Equal positions
and empty moves are allowed. Empty wrappers short-circuit before any pointer
arithmetic or library call, admitting null empty and one-past empty spans; a
nonempty null input is rejected. Bounds failure changes no output. O(count)
time and constant extra storage are required.

`lb_copy` is for separately supplied, caller-proved disjoint live arrays. Its
length/capacity checks do not prove provenance or nonoverlap. Do not compare
arbitrary unrelated pointers with < or > to infer overlap; the original single
array's integer offsets justified that ordering. Do not fix this by assuming
uintptr_t ordering is a portable ownership proof. Alternative explicit owner-ID
and offset designs are acceptable if their provenance contract is derived.
The changed condition is a different proof obligation, not a parameter change.

`lb_strlen` scans no more than the supplied extent and publishes a length only
if it finds a terminator. An empty view has length0, but a zero-extent string
scan has not established a terminator. For bytes65,0,66, the view has length3
and the first string length1. Never infer payload extent from a string function.

`lb_display` preserves a counted binary field by escaping nonprintable octets
and backslash as four ASCII bytes, for example65,0,66 becomes `A\x00B`. It then
adds NUL for the display API. Preflight the actual expansion and terminator with
checked size arithmetic; insufficient capacity changes neither output nor
result. Input/output are disjoint and stable during preflight/replay. This is a
display policy, not a Unicode decoder, HTML escaper or shell quoting function.
The caller still owns all live storage. ISO C does not enforce these promises.

Compose `lb_field_display` with the actual prior FCursor/f_take contract. Read
a big-endian two-octet length and exactly that many payload octets in a private
cursor copy. Commit the cursor only after display succeeds. For00 03 41 00 42 63,
success consumes5 bytes and leaves63 untouched; insufficient input or output
preserves cursor and public output. Do not clone the accepted cursor's code or
call assessor code from a learner implementation. Use the learner's prior artifact.

For the buffer/string variant, reuse the actual earlier lf_prefix and
lf_transaction operations. Derive required6, written0/0/3/6, termination
false/true/true/true and completeness false/false/false/true for capacities0/1/4/7.
Explain the changed temporary-storage/preflight obligation for no truncation.
A finite size check does not establish a string invariant unless a terminator
is present within the live extent.

Compare strcpy, strncpy, snprintf and the counted copy for source lengths3 and6
and capacities3/4. Execute strcpy only when the full source plus terminator fits.
The strncpy limit bounds writes but can leave no terminator; when the source is
shorter, it pads the remaining specified field with zeros. Never strlen its
unterminated result. A fixed-width record instead uses lb_record: reject excess
length, copy exactly n payload bytes, pad all remaining width with zero. A full
record need not be a C string. Source/destination must be disjoint. A zero-width
empty record writes nothing and may use null storage.

Copy each valid int in0,1,-1,INT_MIN,INT_MAX to unsigned-char storage and back
into a declared int; the valid original representation is the premise. An
arbitrary wire pointer cast establishes neither alignment, effective type,
representation validity nor byte order. Decode12 34 explicitly to0x1234 instead.
For the changed struct case, use actual offsetof/sizeof, initialize members,
then explicitly write different known padding bytes. Compare member equality
with memcmp, without inspecting uninitialized padding or claiming a portable
layout. Equal values need not have equal object representations.

Deliver implementations, loop/preflight invariants, exact contracts, the copy
comparison table, byte/metadata guards, retained failures and generated-object
assumptions. Run bytes_tests.c with the actual prior cursor and formatter under
both compilers O0/O2 and hosted sanitizers. The arena oracle uses a complete
before-state snapshot, not the same directional-copy loop. Keep invalid C cases
as reasoning/translation-only; there is no portable golden output for UB.

Rubric:0 violates extent/ownership;1 reproduces the five-byte move only;2 explains
ordering, empty/one-past rules, representation limits and each rollback boundary;
3 independently implements the original separate-array, binary-display,
transactional-output or fixed-record transfer after at least7days. Hints: first
write both remaining extents; next identify the unread source region; finally
locate the first public write and every later possible failure.

All inputs are synthetic local data. This package makes no exploitation or
physical-target claim. Source-bound author execution, independent assessment,
external review and delayed learner evidence remain separate dimensions.
