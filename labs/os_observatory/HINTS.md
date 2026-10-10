# Staged hints

1. Separate a policy's selection rule from accounting. Derive one trace by hand
   before considering loop structure. Completion occurs after the final tick.
2. RR needs a queue-order identity; input index alone unfairly favors low IDs.
   Arrival and requeue at the same instant need an explicit order.
3. For CLOCK, occupancy and reference state are independent. Page zero is valid.
4. Processes need a transport for a child-private result. Shared memory and a pipe
   answer different questions. Close unused pipe ends before waiting for EOF.
5. A signal handler must preserve errno and use only permitted operations. The
   predicate remains authoritative even if a nonblocking self-pipe is full.
6. Retain source/build/environment hashes and raw outputs. Sanitizer resource
   counts describe an instrumented process, not the ordinary build.
