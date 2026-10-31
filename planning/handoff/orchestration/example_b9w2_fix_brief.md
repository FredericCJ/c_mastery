# B9 wave 2: review fixes (comments and local readability only)

Repository /home/fred/dev/c_mastery. Work DIRECTLY in this main checkout. Do
not create a worktree; the wave-2 rewrite is uncommitted here. Edit ONLY the
files named below. Do not commit, and do not touch tools/, curriculum/, research/
or reviews/. Use TMPDIR=/home/fred/.cache/cmastery-tmp/b9w2-fix; /tmp is a
small quota tmpfs. Read curriculum/guides/c-style.md first.

## Hard constraints

- No behaviour change: identical stdout/stderr bytes, exit codes and marker
  strings. A string that a test or tool prints must stay byte-identical, even
  when it contains a typo.
- Do not change any line that contains a mutation macro (`*_MUTANT*`,
  `*_MUTATE_*`, `FD_TRANSFER_*`) or a REQUIRE/CHECK line, unless an item below
  names it explicitly.
- These two files are cited by line number from a curriculum brief. Their
  line count and line positions must not change:
  - labs/foundation_depth/drills/authorization.c (do not edit it at all);
  - labs/foundation_depth/tests/tests.c (edit comment text in place only;
    keep the same number of lines).
- tools/check_object_clinic_contract.py requires these exact declarations in
  labs/object_followthrough/clinic/shared_tests.c. Keep them, whitespace
  aside:
  - `struct mc_shared *extra = NULL;`
  - `struct mc_shared *third = NULL;`
  - `struct mc_shared *excess = NULL;`
  - `context.owner = extra; extra = NULL`
- Learner-visible files (labs/) must not name a specimen's defect, explain
  what a probe will reveal, or describe assessor mutants or controls.
  assessor/ files are private.

## Items: object_followthrough (labs/ and assessor/object_followthrough)

1. assessor/object_followthrough/buffer/buffer.c:35-37. The comment on
   OB_MAX_GENERATION is off by one. The generation starts at 1 and saturates
   at the maximum, so at most MAX-1 invalidating operations succeed. Fix the
   comment. Also clarify labs/object_followthrough/buffer/buffer.h:108 in the
   same way: the generation never exceeds 3, which allows two invalidating
   operations.
2. labs/object_followthrough/alias/alias.h:131. The text "Empty ranges succeed
   (base may then be NULL)" is wrong. Empty ranges whose offsets are within
   length_bytes succeed, and base may be NULL only when length_bytes is 0.
   Confirm against the reference in assessor/object_followthrough/alias/alias.c
   and against CONTRACTS.md.
3. Learner-visible text that documents assessor mutants or controls:
   - atlas/tls.c:11-12: drop the sentence describing OA_TLS_GLOBAL and
     OA_TLS_NOWRITE.
   - alias/target.c:5 and cursor/target.c:5: drop or generalise the
     TARGET_HELPER/malloc sentence.
   - buffer/vla_probe.c:2: "by the campaign" names an assessor-side concept.
4. One declarator per declaration in these local declarations (keep
   initialisers and order; struct member lists in headers may stay):
   - alias/tests.c:274-278 and :380;
   - atlas/tests.c:173 and :269;
   - clinic/tests.c:118-119;
   - cursor/target.c:35;
   - cursor/tests.c:33, :45, :105 and :133.
   Re-check each against the hard constraints first.
5. Comments that are vague or wrong:
   - assessor/.../cursor/loader.c:34: dlerror() is the authoritative check,
     and a NULL table is rejected afterwards.
   - assessor/.../atlas/atlas.c:164: name the two checks, the compile-time
     parameters and the bit pattern of 1.0.
   - atlas/atlas.h:112: "first offset that is a multiple of alignment_bytes".
   - cursor/target.c:2: `oc_read` is called twice, not once.
   - buffer/buffer.h: state that ob_erase is refused once the generation is
     exhausted, and that ob_record_encode rejects a number outside 0..65535.
   - ob_shared_create: give the size_t `n` parameter its unit in the comment
     or the name. Renaming a public parameter in a header is fine; renaming
     a public function is not.
   Verify every new sentence against the reference code.
6. Reflow added comment lines over 80 columns:
   - atlas/atlas.h:68 (113 columns; two sentences);
   - assessor/.../buffer/buffer.c:27;
   - alias/bench.c:58;
   - atlas/atlas.h:123;
   - atlas/tests.c:6;
   - buffer/tests.c:102.

## Items: foundation_depth, effects_depth, numerical_depth

7. labs/foundation_depth/freestanding/entry.c:2. The claim "pulls in every
   public function" is false: fd_verdict, fd_repair_step, fd_from_signed and
   fd_slot_* are not called. Change the comment to list what it pulls in. Do
   NOT add calls.
8. assessor/foundation_depth/transfer/transfer.h:157-158 (fd_registry_init).
   The ownership direction is wrong. The registry does not own its pools:
   each bound pool must stay live and at a stable address until unbound or
   relocated, and the registry itself must stay at a stable address.
9. assessor/foundation_depth/transfer/transfer.h:204 (fd_frame_feed).
   `*committed = 1` means a complete record was applied successfully, even
   when its delta is zero. It does not mean the counter changed.
10. labs/effects_depth/state_test.c:2-3. The CM-SPECIMEN tag lists the hidden
    state categories. Replace it with a neutral tag, for example
    `CM-SPECIMEN: hidden-state probes; observe before explaining (see
    CONTRACTS.md).`, keeping a `CM-SPECIMEN:` marker in the file.
11. assessor/foundation_depth/transfer/transfer.h:118 (fd_compact). Add that
    stream->length becomes the unread count.
12. labs/foundation_depth/include/depth.h:254-259 (fd_apply). Say that a NULL
    counter is rejected first (FD_INVALID), then decoder errors, FD_RANGE and
    FD_DENIED. Confirm against the reference.
13. labs/numerical_depth/numeric.h:133-138 (nd_solve). Add the ND_BAD cases:
    tolerance outside the finite range [0,1), n outside 2..3, and a
    non-square or non-admitted view. Confirm against the reference and
    CONTRACTS.md:190.
14. labs/numerical_depth/fixed_test.c:79-84. The comment says "r" and "p"; the
    variables are `remainder` and `product`.
15. labs/numerical_depth/layout_test.c:42 (`ND_ITEMS - 2`). Name the
    dependency on the two records an earlier check leaves in the store.
16. Learner-visible probes whose comments state the experiment's conclusion:
    - labs/effects_depth/musl_sort_probe.c:5-8;
    - labs/effects_depth/stack_probe.c:4-5;
    - labs/effects_depth/sort_experiment.c ("not a ranking of the
      algorithms").
    Turn each into what to observe or record, without the answer.
17. Where output must stay byte-identical, print through the named constant
    instead of a duplicated literal:
    - assessor/foundation_depth/transfer/comparisons.c:220 (`repetitions=128`
      vs TIMING_REPETITIONS);
    - labs/effects_depth/highwater_test.c:91 (`"pattern":165` vs
      FILL_PATTERN).
    Check that the printed bytes are identical (same value and format).
18. assessor/foundation_depth/transfer/tests.c:503: model_holds_value returns
    `unsigned` for a boolean; make it `int` (or `bool` if the file already
    uses stdbool).
19. labs/foundation_depth/tests/tests.c:39-43: say that the bound
    EXTENT_GRID_MAX is exclusive. Edit in place; keep the line count.
20. Reflow comment lines over 80 columns:
    - assessor/foundation_depth/transfer/comparisons.c:155;
    - transfer/tests.c:50 and 147;
    - transfer/transfer.h:5, 88 and 132;
    - assessor/numerical_depth/interp.c:75;
    - labs/numerical_depth/environment.c:67.

## Verify, then report exit codes

1. Run `build/author-tools/python/bin/clang-format -i --style=file:.clang-format`
   on each changed C file, then
   `python3 tools/check_c_style.py --family object_followthrough --family foundation_depth --family effects_depth --family numerical_depth`.
2. Check that line counts are unchanged:
   `wc -l labs/foundation_depth/tests/tests.c` must still be 699 lines. Check
   that against `git show HEAD:...` plus the uncommitted rewrite: compare it
   with the count before your edit.
3. Run `python3 tools/check_object_clinic_contract.py`. The receipt part may
   fail on stale historical pins; report the "existing tested caller
   sequence complies" line specifically.
4. Build and run the public tests of each touched lab directly, gcc with
   -Wall -Wextra -Wpedantic -Werror, against its assessor reference. Compare
   stdout and exit codes with the same build before your edits. Keep a copy of
   each file before editing so the comparison is exact.
5. Report each item as done or declined (with the reason), with file:line.
