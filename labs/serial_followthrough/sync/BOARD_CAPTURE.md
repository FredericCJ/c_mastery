# Required separate physical dossier

Status: not observed. This file defines the evidence, not a receipt or permission
to connect/flash equipment. Use only a separately authorized dedicated F439 board
and original matched teaching peer, with the exact schematic/revision, part/package,
voltage domains and common reference recorded. PD8/PD9/PD10 AF7 must actually route
to the selected connection; do not assume a virtual COM bridge carries CK.

Record compiled image/source hashes, tool version, USART clock derivation and
BRR, CR1/CR2/CR3, pin configuration, peer response firmware identity, probe type,
instrument sample rate/bandwidth and raw CK/TX/RX capture file hashes. Capture
0x96 and0xA5, an eight-character request and each of the four CPOL/CPHA modes with
LBCL1. Retain preamble, start, data and stop intervals, not a decoder screenshot
alone. Predict edge times and peer data before capture; compare independent raw
edge measurements with the decoded data and exact request sequence.

Measure setup and hold around the selected capture edges against the RM0090
requirement (1/16 bit time for this mode). Explain probe loading and timing
uncertainty. Count all eight captures and show no CK during start/stop. Record
the later TC observation before line release separately from the last DR write.
For the safe changed-condition peer comparison, select LBCL0 only on the
dedicated peer and show its incompatibility with the full-eight-capture contract;
restore the matched baseline before asserting data correctness. A mismatched
phase trace is evidence for diagnosis, not a successful exchange.

No raw physical data are supplied or claimed by the author campaign. If the
device revision/route/peer/instrument differs, state exactly which evidence must
be repeated. A model readback, cross-link or MPS2 execution cannot fill these rows.
