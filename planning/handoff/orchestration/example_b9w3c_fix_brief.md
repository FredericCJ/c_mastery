# B9 wave 3: containers_depth / numeric_followthrough review fixes

Work in the existing worktree
/home/fred/dev/c_mastery/.claude/worktrees/agent-a10d19ba78db8d315. Its
uncommitted diff is the rewrite under review. Edit only files under
labs/containers_depth, assessor/containers_depth, labs/numeric_followthrough
and assessor/numeric_followthrough. Do not commit, and do not touch tools/,
curriculum/, research/ or reviews/. Use
TMPDIR=/home/fred/.cache/cmastery-tmp/b9w3c-fix (never /tmp). Read
curriculum/guides/c-style.md. Keep a copy of every file before you edit it.

## Hard constraints

- Behaviour identical to HEAD: outputs, exit codes, markers, mutation
  macros (CD_MUTANT, NF_MUTANT) and REQUIRE/CHECK lines.
- assessor/algorithm_invariants/instrument.py copies the ring and vector
  functions of assessor/containers_depth/sequence.c and the queue functions
  of transfer.c. assessor/algorithm_heap/generate.py copies ordered.c from
  `static int before(` to `void cd_tree_init(`. Those regions must stay
  TOKEN-IDENTICAL to HEAD. Comments may change.
- labs/ files are learner-visible: no comment that gives away a solution.

## Must-fix

1. labs/containers_depth/containers.h:11-13. The general contract ("a
   non-CD_OK return leaves the container and every promised output
   unchanged; the exceptions ... are named in the function's comment") is
   false. cd_bitset_next returns CD_EMPTY and sets *position (see
   indexed_test.c:236-239); the hash probe counts (cd_hash_put, cd_hash_get,
   cd_chain_get) and the tree-walk prefixes are other exceptions, and the
   header names none of them. Either list every exception in that paragraph,
   or add a one-line comment on each excepted function. Check
   labs/containers_depth/CONTRACTS.md and the reference implementations for
   the exact behaviour.
2. labs/containers_depth/benchmark.c:7. "Every sample verifies its results
   outside the timed region" is false: the CHECKs in run_lookup_samples and
   run_churn_samples are inside the timed loop. Reword accurately.
3. Restore token identity of the copied regions:
   - assessor/containers_depth/sequence.c, cd_vector_destroy: restore
     `v->length = v->capacity = 0;` as one statement;
   - assessor/containers_depth/transfer.c, cd_queue_init: restore
     `q->head = q->tail = -1;`.
   Add a short "copied by assessor/algorithm_invariants/instrument.py; keep
   tokens unchanged" note at the top of sequence.c, as ordered.c and
   transfer.c already have. Prove identity: for every copied function,
   compare the comment-free token stream of HEAD and new with
   tools/c_source.py (`function_span`, `significant(tokenize(...))`) and
   report the result.
4. assessor/containers_depth/sequence.c:643, cd_payload_destroy. Restore
   the HEAD behaviour: v->count decreases before each destroy callback
   (HEAD: `while (v->count) destroy(&data[--v->count])`), so a callback that
   reads the published count sees only the payloads not yet destroyed. A
   named helper is fine, as long as the order of the decrement and the
   callback matches HEAD exactly.

## Should-fix (comments)

5. ordered.c:129: `identity` also holds the member count
   (cd_circle_append increments it, capped at SIZE_MAX).
6. ordered.c:729-730: the node is unlinked when its last occurrence is
   removed (multiplicity 1 at deletion); above 1 it is decremented.
7. transfer.c:126-127: say instead that no callback or cancellation can run
   between publishing the candidate and clearing input->resource (two copies
   do exist in memory).
8. numeric_followthrough/numeric.c:27: the test is a positive range test,
   so NaN, which fails every comparison, is rejected.
9. indexed.c:209-218, cd_hash_rebuild: keep the check of the internal put
   result, with a comment saying it is defensive and unreachable today
   (capacity >= live count, distinct keys, a fresh table has no tombstones).
10. transfer.c:31-33: `fail` is consulted only when the first segment is
    full and refill is requested. sequence.c PAYLOAD_LIMIT: for
    cd_payload_append the limit is on the vector's total.
    payload_test.c:16: the call with that ordinal fails. containers.h:93:
    "capacity 0 -> 1, then doubles" is cd_vector_insert's growth policy, and
    cd_vector_reserve(n) sets any capacity. The cd_vector_shrink and
    cd_vector_reserve comments must name the CD_FULL return when the copies
    counter would overflow.
11. numeric_followthrough/numeric.c: if the mutant branches use the literal
    0x1p-40 while NF_TELEMETRY_* names it, use the macro only where the
    mutant lines are not anchored by tools. Check with
    `git grep -n "0x1p-40" tools/`. Otherwise leave them.

## Verify, then report exit codes

- clang-format -i (pinned, --style=file:.clang-format) on the changed files;
  `python3 tools/check_c_style.py --family containers_depth --family numeric_followthrough`.
- The token-identity proof from item 3.
- `python3 tools/run_containers_followthrough.py` and
  `python3 tools/run_algorithm_invariants.py` in this worktree (they write
  receipts here, which is fine). Read each one's source first: if it needs
  arguments, pass them; never run --help. Then restore research/ and
  remove untracked reviews/ output, as before.
- A behaviour comparison: build the tests before and after your edits (your
  saved copies) and compare stdout and exit codes, including a payload test
  whose destroy callback records v->count.
- Report each item as done or declined, with file:line.
