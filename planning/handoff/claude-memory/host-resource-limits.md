---
name: host-resource-limits
description: Host has ~6.6 GiB usable RAM (8 GB physical) + 24 GB swap and a 3.4 GB quota-limited /tmp tmpfs; limit concurrent builds and keep TMPDIR on disk
metadata:
  node_type: memory
  type: user
  originSessionId: c1d36b06-322f-41b4-8846-6fa7820cb9b2
  modified: 2026-10-30T13:19:42.546Z
---

The development host has 8 GB physical RAM (about 6.6 GiB usable) and 24 GB swap; the user warned to expect swapping. /tmp is a 3.4 GB tmpfs with per-user quota (usrquota) and was exhausted once by campaign temp dirs (EDQUOT broke every Bash call).

**Why:** heavy local work (C builds, ASan/TSan runs, QEMU guests, campaign producers) from several agents at once pushes the host into swap, which slows everything and can make timing-sensitive tests (TSan, PMU measurements) flaky or invalid.

**How to apply:**
- Run at most one campaign driver at a time, and keep build-heavy subagents to about 2-3 concurrent; API-only agents (reviews that mostly read) are cheap.
- Never run PMU/timing measurements while builds or campaigns are running; record load conditions in hardware receipts.
- Set TMPDIR to an on-disk directory outside the repository (e.g. /home/fred/.cache/cmastery-tmp/<task>); a TMPDIR inside the repo makes producers treat temp files as repository inputs.
- /tmp is tmpfs: a reboot (2026-10-30) wiped the session scratchpad, losing briefs and drivers. Keep anything that must survive (worker briefs, campaign drivers, repair scripts) under /home/fred/.cache/cmastery-tmp/ or the repo's build/.
- Related: [[polish-revision-orchestration]].
