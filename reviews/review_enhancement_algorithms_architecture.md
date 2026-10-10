# Independent review of T36/T37 and enhancement guide

Accepted after actual author fixes and fresh rereview. All 20 topics, 60 objectives and 16 detailed briefs have coherent new-work/reuse boundaries. Project hours are 60 for T36 and 58 for T37; each module totals 84 hours without double counting. Exact assigned locators match the accepted source registry.

The teaching distinguishes ISA/ABI, declared pipeline/cache/datapath models and actual MCU/physical evidence. It preserves C17 typed-storage and overflow obligations, requires original callable assembly with actual selected-ISA execution, and does not turn synthetic timing or emulator behavior into hardware/WCET claims. The guide consistently explains these boundaries and earlier-artifact reuse.

Two medium findings were resolved: unsupported no-padding/toy-register/ALU certificates and JSON type confusion. Fresh independent probes now reject all five reproduced corruptions, while baseline fixtures pass. Actual public oracle execution reports 16 fixtures, 2,167 checks and no failures. Two low findings were also resolved: macro/byte-statement prose and explicit A/C/E cards. The JSON receipt retains exact findings, actual commands/results and final file hashes.

Independent arithmetic checks covered all 256 ALU operand pairs per operation, 8,256 bounded heap-child predicates, selected cache/shadow and pipeline schedules, the basic exception/vector offsets, and dimensional CPU/Amdahl calculations. These certify only the stated finite/model expectations. They do not certify a learner C submission, native ABI harness, real board, physical timing, power or WCET.
