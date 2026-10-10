# Runtime assessor implementation

`cvm.c` and `compiler.c` are original complete reference mechanisms for the declared CVM1 subset. They are separate from the unsolved learner sources. Select this directory explicitly with CMASTERY_ASSESSOR_ROOT when validating tests or reviewing a completed learner submission. Do not include it in learner exports.

The reference is an executable oracle for the finite course contract, not a production VM or a proof of all C/runtime properties. The public CTest fixtures are guided anchors. The missing-root mutant deliberately omits explicit roots and must be rejected by the GC test. Require the unmutated implementation to pass first; an arbitrary compiler failure is not a killed semantic mutant.

For a withheld assessment, create a new malformed-bytecode, wrong-frame-base or temporary-root case whose defect and expected result are withheld until the learner submits a frozen diagnosis dossier. Record the seed, initial symptom, independent expected result, artifact hashes and reveal date. The shipped visible mutant does not itself constitute a blind assessment. A learner who has read these sources must reimplement and defend a changed contract before claiming independent mastery.
