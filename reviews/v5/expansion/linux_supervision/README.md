# Linux supervision author evidence

The five T49 exercises run hosted C17 programs in a pinned x86-64 Linux guest under QEMU TCG. The supervisor runs as UID 1000. An observer outside its cgroup and namespaces records kernel state. All runs use a diskless guest with its external network interface disabled. [Learner guide](../../../../curriculum/guides/linux-supervision.md)

| Exercise | What the recorded author run establishes |
| --- | --- |
| X-T49-002 | The observer identified the same live supervisor in two PID views. The child reported its relationship to the supervisor through a pipe. |
| X-T49-003 | With the first worker still alive, a limit of two tasks made the next fork return `EAGAIN` and incremented `pids.events` once. A limit of three admitted that worker. |
| X-T49-004 | The missing executable returned `ENOENT` after the first worker was ready. The supervisor collected both children. In the stubborn-worker trial, GCC recorded 151 ms between TERM and KILL. |
| X-T49-005 | Two independent namespace listeners simultaneously held loopback port 39073. Their replies identified different servers, and a duplicate bind failed in each namespace. |
| X-T49-006 | All three TCP trials conserved 8,192 replies. The quota trial recorded seven throttled periods with GCC and eight with Clang. Each paused-reader trial recorded a refused pipe write and a successful resume. |

[Full campaign and raw serial hashes](current.json) · [Ordinary selected CLI checks and wrong-diagnosis control](selected-cli.json)

Both compilers passed all 12 reference variants and rejected all 12 starter variants. The full campaign also rejected five executable mutations and five altered transcripts. Each of the five separately exported workspaces built and scored both an external author reference and its unfinished starter. To recheck current source bindings and the archived serial, run `python3 tools/check_linux_supervision.py --verify-report reviews/v5/expansion/linux_supervision/current.json` from the repository root.

Two passes through the canonical generators produced identical catalogs, inventory and route outputs. Route regressions verified 2,592 v1 and 2,599 v2 rendered card identities. The v3 route has 250 exercises, with 249 required. [Generator fixed point](generation.json) · [Route and workspace verification](route-verification.json)

All 7,709 members of the candidate learner ZIP matched the public file selection and byte hashes. The course extracted from that ZIP initialized a workspace whose controls built the external author reference and scored all 12 variants. The host processes were denied IPv4 and IPv6 socket creation. TCP loopback within the emulated Linux guest remained available to the exercise. [Archive experiment](learner-zip-guest.json) · [Serial capture](learner-zip-guest.serial.txt)

The original and extracted source trees each passed the 74 public checks and generated 2,634 identical handbook files. The candidate also passed 26 checks of the extracted learner workflows. [Public candidate reproduction](public-candidate.json)

Independent learner assessment and seven-day transfer remain open. These guest observations cover software behavior on the specified image. The course-wide strict evidence gate still reports a stale POSIX launcher export, and the separate binary-inspection family lacks a tracked receipt at the aggregate gate. [Previously recorded strict comparison](../main_integration/strict-author-comparison.json)
