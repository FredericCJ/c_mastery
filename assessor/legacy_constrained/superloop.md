# PR-T30-01 successor analysis

The original6 criteria and6 delayed variants are preserved. One round5 after
slicing is not seven units completed: the saved cursor carries remaining work.
The concrete100-byte run performs50 rounds of2+2+1, so total loop work is250
model units while the parser itself still performs100. The unsliced loop performs
103 total units and blocks every other service during its100-unit dispatch.
Smaller response gaps can increase scheduler/other-service work; never conflate
that change with lower total work or a physical speedup.

A task copies input and stages output. Cursor<=length, READY has unfinished work,
and DONE has exactly one emission. Cancellation does not publish staged bytes.
Repeated result reads copy an already committed result; they do not increment
the emission count. The finite input bound makes unsigned cursor increments safe.
The parser is explicitly ASCII-octet transformation, avoiding ctype locale and
negative-char preconditions while still requiring a complete continuation.

The event FIFO and latest-state slot deliberately preserve different information.
Only the first three command records of the burst are accepted. The latest-state
variant accepts updates but reports four supersessions; it cannot satisfy the
original multiplicity contract. A FULL event push may change only its rejection
telemetry, while a RANGE counter refusal changes nothing. MAX admission counts
must not prevent popping previously accepted work.

Periodic due-count computation uses quotient/remainder bounds: latest<=now,
so next+quotient*period cannot overflow; quotient+1 and aggregate counts still
need explicit checks. Two-slot LOSSLESS can reject a large gap in constant work.
The later smaller model horizon after refusal is only an independent replay,
not a real-time recovery trick. A real clock cannot be rewound to manufacture
capacity. Coalescing and stale rejection commit explicit loss; no-drop workloads
require admission and service assumptions that prevent this situation.

For H budget1/window4, three remaining unit slots serve A/B/C in order. Every
continuously ready ordinary task has service gap4 under exactly these premises.
If H never yields, the scheduler has no authority to enforce its budget. The
bounded observation of blocking.c is evidence of the particular callback path;
the no-return source argument, not a finite timeout alone, refutes a claimed
universal finite service bound. Strict-priority starvation with cooperative H
is a different failure mechanism and is demonstrated separately.

The independent replay uses Python lists and explicit due-release enumeration,
not the core's fixed arrays/closed-form accounting. Its finite domain omits
64-bit extrema, which the direct tests cover. Generated object audits expose
compiler helpers and stack use, but host frame sizes/timing do not establish
MCU resource or WCET properties. No human/learner/physical acceptance follows
from author execution.
