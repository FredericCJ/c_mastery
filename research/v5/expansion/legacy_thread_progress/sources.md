# Bounded source inspections for T25.10–.12

The original briefs, all nine ordinary/changed criterion objects, frozen
waiting/atomic contracts and actual imported implementations were read before
authoring. Bibliography membership is not inspection. Primary passages directly
read on 2026-10-17:

* POSIX.1-2017 `pthread_mutexattr_getprotocol`, man7 reprint: DESCRIPTION
  lines31–108 and ERRORS117–129, including recursive inheritance50–104.
* Linux `sched(7)`: API86–122, FIFO124–171, RR172–182 and SCHED_OTHER273–288.
  Its historical introductory scheduler description is not used to identify
  this kernel's current implementation. No scheduling setting changed.
* Linux man-pages `clock_gettime(3)`, release5.07/2020: monotonic-clock91–117,
  CPU-clock distinctions130–137 and return values161–165. Equal monotonic
  readings are possible; observed maxima are not scheduler guarantees.
* Herlihy and Wing1990, *Linearizability: A Correctness Condition for Concurrent
  Objects*, institutional primary PDF. Read opening pages463–467 through web
  rendering, then directly extracted/read printed468–470 (PDF indexes5–7)
  with pypdf after repeated web section timeouts. These contain sections2.1/2.2
  and queue examples2.3: histories, pending completion, equivalence and real-time
  precedence. No whole-proof inspection is claimed. Private source cache:
  `/home/frede/cmastery-threads-sources/herlihy-wing90.pdf`, SHA256
  `79e2ff81a15c55de870492231e81e36c50366389c118a7317b5877f603cfa2a0`.
  The copyrighted PDF/extracted passage is not published with the learner work.
* Linux kernel documentation `kernel-hacking/false-sharing.html`: complete
  article read; specifically mechanism1–36, pitfalls38–50, mitigation77–118.
  No kernel code is imported and no perf-c2c observation is fabricated.
* WG14 N1570, C11 committee draft5.1.2.4 paragraphs1–28 and7.17.3 paragraphs1–10,
  directly inspected in the immediately preceding atomic wave and referred to
  for the imported restricted publication/reuse model. It is not final C17.

Exact final C17 and published C23 remain unavailable. POSIX.1-2024 is not
substituted for the inspected2017 protocol. The host reports compiler/libc/kernel
identities at execution. No hardware traffic, physical weak-order, ISR,
external-review or delayed-learner result is claimed.

The first actual campaign failed strict compilation because include sorting
placed a system header before the imported feature-test macro. The failed
receipt and original outputs are retained as `attempt01_feature_macro.json`.
The corrected test declares `_POSIX_C_SOURCE` before all headers. Later
campaigns use unique build directories; failed outputs are not overwritten.
