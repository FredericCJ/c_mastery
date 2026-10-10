# Hosted-system contract

All descriptor arguments are borrowed unless a function returns a newly owned descriptor. Callers provide exclusive reader access while a deadline or reactor operation runs; otherwise readiness may be consumed by another thread before a blocking read. Output buffers remain caller-owned. The copy accepted-prefix count is authoritative on error; input may already have advanced farther than output. Frame failure leaves output length unchanged, but a partial body may already have modified the destination. Close the input on framing failure.

Fork-based exercises run before creating additional threads. The mapping is a real MAP_SHARED region of the stated extent, and parent reads follow successful child reaping. Its offset/length header uses target-native uint32_t for this finite exercise, not a cross-endian wire format. The signal exercise exclusively owns SIGUSR1/SIGUSR2 with no preexisting pending instance and runs in a single-threaded process. External signal senders are outside the finite test.

Pool count is at most 256 and worker count is 1..4; input stays immutable through joins and output is published only on success. Linux x86-64 long can represent the sum of 256 int values. Queue storage is four integers. Close is idempotent, rejects new pushes, wakes all blocked users and permits FIFO draining. Join all users before destroying mutexes or condition variables; ordinary thread cancellation is excluded.

Seal the pool design choice before coding: (a) immutable static partitions or (b) a shared next-index protected by a mutex. Both assessor alternatives pass the same observable contract. Predict behavior on uneven work and justify whether the additional shared coordination is useful. On later changing jobs to unequal-cost tasks, revisit that decision.

Clock observations are measurements, not scheduler guarantees. File-size limits are changed only in a child, SIGXFSZ is handled there, and the parent limits must remain unchanged. No cgroup, privileged scheduler, storage durability or real-time guarantee follows from these tests.

The file-limit exercise receives an empty regular file positioned at offset zero. Deadline instants earlier than or equal to the observed monotonic instant expire immediately, including negative inputs; the implementation must not overflow when computing the wait duration.
