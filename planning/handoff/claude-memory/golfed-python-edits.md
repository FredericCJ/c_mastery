---
name: golfed-python-edits
description: "Many c_mastery tools are code-golfed one-liners; never append an inline # comment when patching part of such a line"
metadata:
  node_type: memory
  type: feedback
  originSessionId: c1d36b06-322f-41b4-8846-6fa7820cb9b2
  modified: 2026-10-30T14:28:17.883Z
---

Many tools/ and labs/ Python scripts put whole function bodies on one line,
joined with `;`. Appending `  # comment` after a replaced expression comments
out every statement that follows on that line.

**Why:** in this revision (commit 1f13170b6) an inline comment silently removed
`rows.append(...)` and `return ok` from 14 export checkers' `run()`, so every
`cmastery init` looked failed; fixed in 3e9fe307b.

**How to apply:** when patching golfed lines, change only the expression; put
any explanation in the commit message or on its own line above the def. After
patching, check that nothing follows the replaced text on the same line, and
run the tool once. Related: [[c-house-style]].
