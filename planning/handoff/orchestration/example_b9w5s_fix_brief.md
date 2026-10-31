# B9 wave 5: posix_context_depth / service_depth review fixes

Work in the existing worktree
/home/fred/dev/c_mastery/.claude/worktrees/agent-a131ca9693b11c482. Its
uncommitted labs/ and assessor/ diff is the reviewed rewrite. Edit only files
under labs/posix_context_depth, assessor/posix_context_depth,
labs/service_depth and assessor/service_depth. FIRST restore the receipts
the producers modified: `git checkout -- reviews research`, then delete the
untracked reviews/v5/expansion/followthrough/service_adapters/2026103*/
directories (list them, then remove exactly those two paths, no globs over
other directories). Do not commit and do not touch tools/. Use
TMPDIR=/home/fred/.cache/cmastery-tmp/b9w5s-fix. Keep copies of the files
before editing.

## Must-fix

1. labs/posix_context_depth/context.h:50 (learner-visible): `visible` is 1
   once this call's rename completed; a later writer may still replace the
   name (CONTRACTS.md; storage.c locking() replaces it with "rogue").
2. labs/posix_context_depth/tests/helper.c:22-23: the "forked-but-not-
   exec'd copy" argument is false. Say: "Initialized data of this image,
   reported as the new image's private initialization (CONTRACTS.md)."

## Should-fix

3. assessor/service_depth/kernel/services.inc, sd_queue_receive (about
   :826 and :889): the rewrite stores `partial` before sd_wake(q), but HEAD
   re-read f->rax after sd_wake. Under SDK_MUTANT_CANCEL a cancelled thread
   keeps active=1, so sd_wake can overwrite the current frame's rax, and the
   two versions then differ (HEAD returns 3; new returns 65539 for
   SDK_RECEIVE_PARTIAL length 3 after a cancel). Restore HEAD semantics
   exactly: produce the return value from call->frame->rax AFTER sd_wake, as
   HEAD did. Check HEAD's code (`git show HEAD:assessor/service_depth/kernel/services.inc`)
   and mirror its order.
4. labs/posix_context_depth/context.h:98: "a positive injected_error is
   returned unchanged" (a negative value gives EINVAL).
5. context.h:75: replace "pc_remaining saturates at 0 after the deadline"
   with the CONTRACTS wording "returns the nonnegative interval length"
   (do not give away the subtraction).
6. services.inc:5-6: "is entered with the kernel metadata lock held" (SDK_CUT
   calls block(), which drops the lock while idle).
7. load.inc:5-6: -1 is also returned when the archive has no entry with
   that name.
8. labs/posix_context_depth/tests/timing.c:228: a relative restart
   extends the deadline (CONTRACTS: "extends the contract"); reword.
9. Nits:
   - context.h:66: remove the stray line break after "A name";
   - context.c:339 pc_spawn: pc_launch also returns 0 with kind 2, and the
     kind-2 path also covers file-action setup failures;
   - context.c:379 `temp_created` stays 1 after the rename; describe it
     accurately;
   - load.inc:27: name PF_R (4) instead of a bare 4 (check the object code
     is unchanged);
   - service_depth.c:923: the caller does get SD_UNCERTAIN.

## Verify, then report exit codes

- Pinned clang-format -i, then
  `python3 tools/check_c_style.py --family posix_context_depth --family service_depth`.
- Item 3: build a host harness or reason from the code, and show that new
  equals HEAD for the cancel-then-partial-receive scenario under
  -DSDK_MUTANT_CANCEL, and that the non-mutant path is unchanged.
- `python3 tools/run_unit_campaigns_v5.py family service_depth` in this
  worktree, with CMASTERY_TIMEOUT_SCALE=4. Read the source first; never run
  --help. Then `git checkout -- reviews research` and remove the untracked
  service_adapters/<stamp>/ directories it creates again.
- Build and run the posix_context_depth tests directly (the producer has
  /home/frede paths): gcc and clang, -O0 and ASan+UBSan, all 5 cases; PC
  mutants 1-5 rejected.
- Report each item as done or declined, with file:line, and confirm
  `git status` shows only labs/ and assessor/ changes.
