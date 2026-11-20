# Supervise a service in a Linux guest

Implement a hosted C17 supervisor and compare its account of each process or
socket with observations made outside its namespace. The fixture supplies a
disposable x86-64 Linux guest running under QEMU. It controls namespaces,
cgroup v2 limits and loopback setup. Your supervisor runs as UID 1000.

| Exercise | Project | Case | Investigation |
| --- | --- | --- | --- |
| X-T49-002 | PR-T49-08 | `namespace` | Match a process seen from two PID namespaces and compare its mount and cgroup views. |
| X-T49-003 | PR-T49-09 | `pids` | Change one task limit and explain the outcome of a second fork. |
| X-T49-004 | PR-T49-02 | `shutdown` | Account for every pipe and child during successful startup, partial failure and shutdown. |
| X-T49-005 | PR-T49-08 | `network` | Bind the same loopback address in two network namespaces and identify the listener reached by each client. |
| X-T49-006 | PR-T49-09 | `telemetry` | Diagnose CPU throttling or downstream backpressure for a fixed request batch. |

The first four exercises are required on `contextual-v3`. Start after
`U-T49-08-03` and `U-T49-09-04`. The network exercise also requires
`U-T24-02-04`. The telemetry exercise is elective and becomes ready after all
four core exercises. Existing profiles can review and explicitly adopt the
route with the study CLI. [Route and migration](../v5/routes/README.md)

## Set up the first workspace

From the course checkout or learner archive, list the selections and export the
first case. Use a fresh destination for each exercise.

```sh
python3 -m cmastery list --module T49 --exercises
python3 -m cmastery init PR-T49-08 ../supervise-pid --exercise X-T49-002
cd ../supervise-pid
python3 guest.py setup
python3 guest.py smoke --out build/smoke-1
```

The runner uses a Linux x86-64 host with Python, a compiler and static C
library, CMake, Ninja, binutils, dpkg-deb, cpio and QEMU. Setup downloads a
checksum-pinned Ubuntu kernel and static BusyBox into the user cache. Each
subsequent boot uses those cached files. Use a fresh output directory for each
attempt. If a prerequisite is missing, the runner records the unavailable
capability. [Ubuntu setup](public-reproduction.md)

## Make and check a prediction

Read `ACTIVE.md`, `CONTRACTS.md` and `PROTOCOL.md` inside the workspace. For
the first case, write your prediction in `answers/namespace.json` and
implement `supervise_namespace` in `src/namespace.c`. Then build and observe.

```sh
cmake -S . -B build/learn -DCMAKE_C_COMPILER=gcc
cmake --build build/learn
python3 submission.py observe namespace --program build/learn/supervise --out build/first-namespace
```

Compare the serialized inner and outer observations in
`build/first-namespace/guest/serial.txt`. Keep your original prediction and
add the explanation requested by the card. From the course directory, run
`python3 -m cmastery check ../supervise-pid --compiler gcc --profile debug`
to obtain a receipt for a fresh run. Use the same sequence with the project,
exercise ID and case from the table for each later workspace. The card’s
rubric covers your reasoning and the changed-condition assessment.
