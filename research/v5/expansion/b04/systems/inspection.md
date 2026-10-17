# Second B04/B05 mechanism wave

Read UM10204 revision7.0 sections3.1.5/3.1.6/3.1.8/3.1.9, RFC1071 introduction
and section2(A), and the Linux generic MMU-gather ordering comment (lines27–41
of the fetched master snapshot). resources.json records response-byte hashes.
These are bounded actual passage inspections, not claims about complete manuals
or inspected MCU/ISA register behavior. The moving Linux source is identified
by fetched bytes; no universal current-kernel guarantee is inferred.

The nine new mechanisms add five-stage timing under a declared model, sampled
I2C arbitration/stretching, streaming checksum parity, paired-sample statistics,
tri-valued token reduction, FIFO/LRU/offline-optimum comparison, atomic wait
enrollment, shootdown acknowledgement and peer restart uncertainty. Each has
a concrete C API, compiling failure scaffold, separate reference, adversarial
test and compiled mechanism-specific mutant. Independent mathematical methods
cover checksum segmentation, offline victim-choice DP, paired medians and all
eight-CPU target masks; the report states this scope explicitly.

Existing source inspection that motivated this wave is recorded in the parent
inspection notes: the architecture ALU/cache tests are real and retained;
OS-observatory CLOCK already discriminates FIFO; accepted communication and
execution libraries provide richer integration boundaries. The new work does
not claim these mechanisms were absent. It supplies missing independent or
changed-assumption exercises, with native integration still separately assessed.

No synthetic cycle, duration, page ID, cache state, bus sample or completion
event is reported as an observed physical quantity. No target/board run, human
reviewer independence or delayed learner assessment is claimed. Full unsampled
leaf review and corpus expansion remain active, recorded in the B04/B05 ledger.
