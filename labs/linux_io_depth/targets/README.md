# Local Linux observation and completion adapter

Compile `observe.c` with the recorded system headers and compiler and run in a
disposable local directory. It uses anonymous pipes, its own signal, descriptors
and a temporary file. It restores its process signal mask; it changes no host
security setting. A normal run removes its own file/directory; if interrupted,
remove only the printed or inspected `cm-b06-*` directory belonging to that run.

The probe demonstrates the edge-triggered partial-drain boundary, local eventfd,
timerfd and signalfd, file plus directory fsync calls and sendfile data transfer.
A syscall return is not physical power-loss validation or zero-copy proof.
io_uring setup is only a capability probe. Implement raw-ring submission and
completion ownership in learner code, including release/acquire publication,
short I/O, cancellation success/ENOENT/EALREADY, timeout, multishot termination
and a recycled slot generation. Preserve all payloads until the target terminal
event is consumed. An unavailable backend remains a required experiment gap.

Add writev progress over zero-length and partially written vectors, splice pipe
ownership, close races and allocation failures. Predict outcomes first, use
bounded local operations and retain exact logs. No external service is a target.
