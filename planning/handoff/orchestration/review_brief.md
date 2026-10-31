# Review brief: B9 wave 2 readability rewrite (independent review)

Repository /home/fred/dev/c_mastery. Read AGENTS.md, CLAUDE.md and
curriculum/guides/c-style.md. The change under review is the uncommitted diff
in the main checkout against HEAD:

    git -C /home/fred/dev/c_mastery diff HEAD -- <your paths>

Ignore untracked files (assessor/board_f439, tools/f439_board.py and others are
a different batch). This is a READ-ONLY review: do not edit any repository
file, do not commit. Scratch work goes in TMPDIR=/home/fred/.cache/cmastery-tmp/
review-b9w2-<your scope> (create it; /tmp is a small quota tmpfs; never put
scratch inside the repository). The host has ~6.6 GiB RAM shared with other
jobs: build one thing at a time, `make -j2` at most.

## What the change was supposed to do (the contract)

A semantic readability rewrite of lab families, on sources that were already
renamed and clang-formatted:

- split dense functions into named steps; remove accidental complexity;
- one declarator per declaration, smallest scope, `static` file-local helpers,
  `const` for unmodified objects;
- named constants with units for non-obvious magic numbers;
- comments state invariants, ownership, error contracts and why; public
  functions get contract comments (preconditions, ownership, return value on
  each failure, blocking);
- argv/env numeric parsing through labs/support/cm_parse.h, except in
  deliberate specimens.

It must PRESERVE:

- behaviour and public contracts: outputs, exit codes, marker strings that
  tests/tools match, mutation-catalog anchors;
- learner scaffolds stay unfinished (stubs return their sentinel);
- deliberately defective specimens (files tagged `CM-SPECIMEN:`) keep their
  defects;
- learner-visible files (everything under labs/) must not leak answers: no
  comment that names a specimen's defect, explains what a probe will reveal,
  or gives away a scaffold's solution. assessor/ files are private and may
  explain freely.

## What to do

1. Read the diff of your scope file by file.
2. Differential testing: for every rewritten pure or deterministic function
   (decoders, encoders, arithmetic, partitioning, index math, parsers), build
   the HEAD version (`git -C /home/fred/dev/c_mastery show HEAD:<path>`) and
   the new version side by side in TMPDIR and compare them on a large input
   set (exhaustive where the domain is small, otherwise millions of random
   plus boundary cases). Include sanitizers (-fsanitize=address,undefined)
   in at least one build. Report the case counts and any divergence with a
   concrete input.
3. Check every new or changed comment against the code it describes: a
   contract comment that misstates a precondition, failure value, ownership
   or unit is a defect. Check named constants have the value and unit the
   name claims.
4. Check that specimens still have their defect, scaffolds are still stubs,
   and learner-visible comments leak nothing.
5. Check cm_parse use: the right parser for the type, range checked before
   narrowing, error exit preserved or improved, and nothing changed in a
   specimen.

## Report

For each finding: severity (must-fix: behaviour/contract/leak/wrong comment;
should-fix: readability or clarity; nit), file:line in the NEW source, the
concrete failing input or the exact misleading sentence, and a suggested fix.
Then: what you tested (functions, case counts, sanitizer builds, exit codes)
and what you could not verify. Do not restate the author's claims as findings;
verify everything yourself.
