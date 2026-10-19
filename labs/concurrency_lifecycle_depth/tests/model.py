"""Independent finite queue model; 6^5 traces, including errors after close/abort.

This explores these discrete inputs only; it is not a concurrency or C proof.
"""
import itertools
import subprocess
import sys

commands = []
expected = []
for trace in itertools.product('ABrRCX', repeat=5):
    commands.append('Z' + ''.join(trace))
    queue = []
    closed = False
    accepted = delivered = discarded = 0
    for operation in trace:
        status, n, producer, sequence, first, second = 0, 99, 99, 99, 99, 99
        if operation in 'AB':
            if closed:
                status = 4
            elif len(queue) == 2:
                status = 2
            else:
                queue.append(operation)
                accepted += 1
        elif operation in 'rR':
            if not queue:
                status = 4 if closed else 1
            elif queue[0] == 'A' and operation == 'r':
                status = 3
            else:
                value = queue.pop(0)
                delivered += 1
                n, producer, sequence = (2, 1, 1) if value == 'A' else (0, 2, 2)
                if value == 'A':
                    first, second = 11, 22
        elif operation == 'C':
            closed = True
        else:
            discarded += len(queue)
            queue = []
            closed = True
        expected.append((status, len(queue), accepted, delivered, discarded,
                         n, producer, sequence, first, second))
result = subprocess.run([sys.argv[1]], input='\n'.join(commands),
                        capture_output=True, text=True, timeout=20)
if result.returncode:
    raise SystemExit('Trace executable failed: ' + result.stderr)
actual = [tuple(map(int, line.split())) for line in result.stdout.splitlines()]
if actual != expected:
    differing = next((i for i, pair in enumerate(zip(actual, expected)) if pair[0] != pair[1]), None)
    raise SystemExit('CL_CHECK: finite queue model mismatch at row ' + str(differing))
print('CL_MODEL_PASS traces=7776 operations=38880')
