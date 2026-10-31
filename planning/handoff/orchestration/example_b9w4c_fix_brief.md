# B9 wave 4: checkpoint_* / integration review fixes

Work in the existing worktree
/home/fred/dev/c_mastery/.claude/worktrees/agent-aa55ea3309e397f26. Its
uncommitted diff is the reviewed rewrite. Edit only files under
labs/checkpoint_followthrough, assessor/checkpoint_followthrough,
labs/checkpoint_linux_followthrough, assessor/checkpoint_linux_followthrough,
labs/integration and assessor/integration. Do not commit, and do not touch
tools/, curriculum/, research/ or reviews/. Delete the stray untracked
reviews/v5/expansion/integrated/new_family_acceptance.json in the worktree.
Use TMPDIR=/home/fred/.cache/cmastery-tmp/b9w4c-fix (never /tmp). Keep copies
of the files before editing.

Constraints: behaviour identical to HEAD except item 1 (which restores HEAD's
behaviour). Mutation macros (P5_MUTANT, CP_MUTANT, CPX_MUTANT, IN_*),
REQUIRE/CHECK/CPA_ASSERT lines and the v4_mutations.json anchors in
assessor/integration/integration.c stay unchanged; anchors match raw text
including comments, so do not quote anchor text in comments.
tools/check_checkpoint_protocol_exports.py matches the stub text
`CPA_ASSERT:cpa_admit(&p,&q,capacity)==CP_OK` (whitespace-insensitive);
keep it.

## Must-fix

1. assessor/checkpoint_followthrough/entry/image.c:115-117, test_code_of:
   it reads entry[sizeof name] even when entry[sizeof name - 1] is 0, one
   byte past "CE_TEST=" (an ASan heap-buffer-overflow in a hosted harness;
   HEAD read it only when the value byte was non-zero). Add
   `if (value == 0) { return 0; }` before reading the terminator, or the
   equivalent that matches HEAD exactly.
2. image.c:67: cpw_copy is defined by the aggregate probe object
   (labs/checkpoint_depth/tests/helper_probe.c); byte_helpers.c provides
   memcpy/memset. Fix the comment.
3. labs/checkpoint_linux_followthrough/bridge.h:59-61 (learner-visible),
   p5_bridge_cancel: it can also return 0 (begin, cancel=1, wait=0,
   observe=0, then cancel of the same ticket before reap returns 0). It can
   return -1 after recording the cancel request when the wake-pipe write
   fails. State both, checked against bridge.c.
4. assessor/.../bridge.c:155-157, p5_bridge_init: *p is not written if fd
   is invalid or blocking (fcntl F_GETFL failure or a blocking descriptor
   returns -1 early); later failures leave initialized == 0.

## Should-fix

5. bridge.h:86-91, p5_bridge_destroy: -1 after a late close error still
   leaves the bridge destroyed (initialized=0, fds -1; failures.c asserts
   this), while -1 after an in_shutdown, cond-destroy or mutex-destroy
   failure leaves it initialized. Say which is which.
6. image.c:246-250: "the writer never blocks" is false (a raw write to a
   blocking stdout pipe can block); the 16-turn limit bounds retries and
   partial writes.
7. ownership.c:5-7: the shared cpa_queue (with its own mutex) is also
   shared mutable state.
8. bridge.c:7-8: pthread_create runs under the mutex in begin, and the
   design assumes the caller keeps O_NONBLOCK on the shared open file
   description. State both.
9. replay.c parse_arguments: add one line saying why it uses a local
   checked parser, not labs/support/cm_parse.h (the learner export build in
   tools/check_checkpoint_protocol_exports.py has no -Ilabs/support and does
   not link cm_parse.c). Use `!= 0` on the int return. decode_hex comment:
   `count`, not `hex_chars/2`. REPLAY_TURN_FACTOR: drop the unfounded "at
   most one feed turn" claim; say the bound only needs to be generous.
   Optionally derive REPLAY_MAX_INPUT_BYTES as REPLAY_MAX_HEX_CHARS / 2.
10. writer.c: add a contract comment to category_of, and rename `enum
    writer_limit` to something accurate, since it holds WRITER_RETRY = -2
    and CAUSE_ADAPTER_FAILURE = 1001. It is file-local; check that no tool
    references it. writer.h cwf_byte_step: in_drain also returns -1 for a
    queue value outside 0..255.
11. labs/integration/include/integration.h:
    - in_shutdown: `*discarded` is incremented and never reset, so the
      caller initialises it;
    - in_filter: out[] and the filter history may be partly advanced on a
      component failure;
    - in_vm_store: -1 also for a null page or cap < STORE_PAGE_SIZE;
    - in_gateway: -1 also for a full queue or an unhealthy watchdog.
    integration.c: give VM_FUEL and VM_GC_BUDGET units.
12. Nits: bridge.h:52 "before begin" means "while no worker is started"
    (offer works again after reap). waiter.c:19: the 5 s bound counts only
    sleep time. image.c: CE_FAIL_STACK also covers the .bss/.data sentinels
    and argc, CE_FAIL_ENVIRONMENT also covers auxv and page size, and
    check_entry_state's comment omits the argv NULL check.

## Verify, then report exit codes

- Pinned clang-format -i on the changed files, then
  `python3 tools/check_c_style.py --family checkpoint_followthrough --family checkpoint_linux_followthrough --family integration`.
- For item 1: build image.c in a hosted harness with
  `-fsanitize=address,undefined`, each env string malloc'd at its exact
  size, including "CE_TEST=", and show the overflow gone. Compare outputs
  with HEAD over a few hundred crafted cases.
- `python3 labs/integration/tests/v4_mutation.py`-style anchor check: every
  anchor in assessor/integration/v4_mutations.json matches once (use the
  script's own anchor_spans).
- Run in this worktree:
  - run_checkpoint_protocol_followthrough;
  - run_checkpoint_writer_followthrough;
  - run_checkpoint_linux_followthrough --report <fresh path>;
  - run_capstone_linux;
  - check_checkpoint_protocol_exports, check_checkpoint_writer_exports and
    check_checkpoint_linux_exports.
  Read each one's arguments in its source; never run --help. Then restore
  research/ and remove untracked reviews/ output.
- Report each item as done or declined, with file:line.
