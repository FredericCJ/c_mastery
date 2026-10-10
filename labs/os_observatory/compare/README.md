# Same workload, three execution environments

The original execution family, Linux pthread application and FreeRTOS application
send 256 ordered unsigned values (0..255) through eight message slots. Acceptance
requires count256, checksum32640, no duplicate/loss and complete draining. Linux
uses channel close; FreeRTOS uses an explicit end-of-stream message because an
RTOS queue does not supply the same close API. Compare that responsibility too.

Learner comparison mains are unsolved. Assessor implementations are separate.
The FreeRTOS adapter is a hosted comparison using libc/pthreads/signals through
the POSIX port; it is not a strict image or a physical RTOS timing measurement.
Its elapsed and process CPU times include the selected port and environment.

Provide an explicit official FreeRTOS-Kernel V11.2.0 checkout (commit
`0adc196d4bd52a2d91102b525b0aafc1e14a2386`) and configure:

```sh
cmake -S labs/os_observatory -B build/os-compare -G Ninja \
  -DCMASTERY_FREERTOS_ROOT=/path/to/FreeRTOS-Kernel
cmake --build build/os-compare
ctest --test-dir build/os-compare -R comparison --output-on-failure
```

The learner runner also accepts the declared comparison target:

```sh
python3 -m cmastery check ../my-comparison --target freertos-posix \
  --external freertos=/path/to/FreeRTOS-Kernel
```

This checks the pinned Git commit, rejects dirty dependency trees and hashes the
external source closure into the receipt. Missing external sources yield
`unavailable`; they never cause an unregistered RTOS test to count as passed.

Pin the checkout/hash and record the exact configuration and commands. Kernel
sources are not silently downloaded or included in the strict runtime. Static
task allocation in this configuration does not imply that the POSIX port or
pthread implementation performs no allocation. Inspect port.c and its generated
dependencies. The F439 FreeRTOS comparison remains a separate physical project.
