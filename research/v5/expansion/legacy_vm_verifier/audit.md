# T46.02 exact successor audit

The original three criteria have substantive baseline verifier/VM anchors, but
that evidence does not implement their SPLIT lowering, initialized-local lattice,
late-path reprocessing or complete paired malformed/load variants. This successor
retains the original criteria and adds those mechanisms, independent path-set
oracles, strict target closure and separate assessor transfer material.

Inspected: actual original CVM specification/header, width/verify/decode/load/run
implementations, original PR-T46-02 task/fixture, and the named primary Clang
passages in resources.json. The original CVM provider remains unchanged. SPLIT
uses an explicit selector and lowering, not a silent change to CVM1 dispatch.

The pilot exposed excessive conservatism in summing four mutually exclusive test
entry frames. The successor audit explicitly uses their maximum and sums every
common live callee, with a1024-byte reserve. No target stack reservation or
mandatory check was reduced. The selected entry contains no recursive C call or
arbitrary host callback; compiler stack records and final imports remain required.

Independent review, learner attempts, seven-day transfer and physical target
observations remain separate, as do all other required T46–49 obligations.
