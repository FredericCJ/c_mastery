# VT8 withheld assessor analysis

The target writes state during the parse, ignores duplicate keys and never checks
the final extent. Repair must stage an entire five-byte state and commit only
after all records and the exact end validate. A four-bit seen set enforces the
specified uniqueness policy; clearing value on deletion makes its representation
contract explicit. The byte-extent invariant is at<=n before any n-at subtraction.
Each read follows admission of its complete record header/payload; maximum four
records bounds work even on oversized inputs. A separate parsed-record oracle
first constructs an abstract operation list and checks unique keys before applying
it, so it is not a line-for-line copy of the incremental C implementation.

The bounded proof establishes memory-safety instrumentation and unchanged state
on failure for arbitrary22-byte buffers and n<=22; it does not prove acceptance
semantics. Differential records and malformed variants address acceptance but
remain finite. Record each mutant's first input and property. The original flawed
target is safe to run within this bounded local teaching setup because it checks
payload extents; its demonstrated violation is transactional/canonical policy,
not an inference of arbitrary memory corruption.

VT8-DELAYED removes only uniqueness checking. The learner must revise the oracle
and explain why previous duplicate-key failures become valid while malformed
later records and tails still roll back. An answer copied from this file is
assisted evidence, never the independent criterion requested by the parent.
