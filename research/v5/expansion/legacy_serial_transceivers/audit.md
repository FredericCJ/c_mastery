# T38.07–.08 actual source and depth audit

The full retained PR-T38-06/07 contracts and all six original criterion/V1
objects were read. The actual serial_depth electrical tests only classify a
few synthetic thresholds/topologies; they do not implement selected circuit
admission, typed connector roles, capture completeness, common-mode accounting
or multipoint lease design. The new successor supplies those mechanisms and
composes the actual frozen UaTx/PuDriver/SDUart implementations. Existing
accepted identities and historical receipts remain unchanged.

Primary TI PDF passages were read from the actual web documents; exact copies
were privately cached and selected diagrams rendered/viewed locally. MAX3232
SLLS410O June2021 pp3–6,10–12 distinguish package pins, operating ratings,
guaranteed and typical thresholds/rate, inversion/open state and the circuit
capacitors. Figure9-1 was viewed. AM26LV31E SLLS848C April2024 pp3–6,10–13
provide the selected driver pins, operating limits, truth table and termination
discussion. AM26LV32E SLLS849E August2023 pp2–6,8–9 provide receiver polarity,
common-mode/differential limits, open-only fail-safe, disabled state and the
revision correction removing shorted/terminated fail-safe. Both pin diagrams
were viewed. No full TIA standard, arbitrary connector or commercial adapter
was inspected or claimed interchangeable.

The AM26LV31E pin table calls VCC a5V power input while its actual recommended
operating table specifies3.0..3.6V. The successor records the discrepancy and
uses the operating table; it does not silently treat the isolated pin label
as permission for5V. The application text also contains generic RS485 examples;
that does not transform this selected one-driver RS422 circuit into a validated
multipoint circuit. The changed coordinator is explicitly an original trusted
software exclusion policy with separate physical prerequisites.

The current bounded GCC pilot executed six selectors successfully, including
50001 voltage decisions,28002 differential/sign decisions,256 complete UART
frame captures, actual16-byte/two-chunk transmit composition,27 wiring triples
and finite ownership/capture falsifiers. These are author pilots; the final
source-bound report separately records exact actual profiles/target execution.
Synthetic raw line points are not measured waveforms. Physical line amplitude,
two receiver captures, changed cable/load/baud measurements, source uncertainty,
independent delayed learner performance and external review remain required.
