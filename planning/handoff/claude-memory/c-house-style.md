---
name: c-house-style
description: "User-mandated C house style for c_mastery (BARR-C/CERT/MISRA/kernel blend, clang-format enforced, public rename)"
metadata:
  node_type: memory
  type: project
  originSessionId: c1d36b06-322f-41b4-8846-6fa7820cb9b2
  modified: 2026-10-29T13:38:17.940Z
---

All active C in c_mastery (labs/, assessor/) is rewritten to a house style blending BARR-C:2018, SEI CERT C, MISRA C and Linux kernel style, enforced with pinned clang-format 21.1.8 (decided 2026-10-29).
Layout: BARR-C Allman braces, 4-space indent, 80 columns, mandatory braces. Public names: lowercase module-prefixed `struct`/`enum` tags, no struct typedefs (resolves BARR-C `_t` vs POSIX reservation). The six genuine Linux kernel modules keep kernel style.

**Why:** the user wants readability and an introduction to industry standards; correctness/safety rules apply only in some contexts (freestanding/firmware, security).

**How to apply:** follow `curriculum/guides/c-style.md` once written; MISRA is cited by locator from the user's licensed library at /home/fred/library/MISRA (never copy its text into the repo). See [[polish-revision-orchestration]].
