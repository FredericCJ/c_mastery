# Readiness, lifetime and shutdown: current author validation

Receipt: `reviews/v5/expansion/legacy_reactor/current_validation.json`

SHA256: `41eeae12968d515875d248741b1870d97793da6dbd648e613746ee7c3b62d265`

Observed: 134 exact commands, 18 ordinary/changed observations; campaign passed=True. 25 report controls passed. Actual CLI export passed=True; 0 additional standalone export commands where supplied. Original criterion bodies and seven-day delays are preserved by the integration receipt.

Fixed two discovered lifetime defects: nonreused per-operation completion tokens distinguish a repeated old CQE from the next operation, and cancelled messages remain owned until final callback return. Serialized completion-event tests are separate from required T24.12 kernel I/O.

Every compiler invocation, result, environment, source identity, raw output, expected rejection and retained artifact is in the JSON receipt. Native GCC/Clang debug/optimized/ASan+UBSan profiles were actually run; reactor/timers additionally run both TSan profiles with a known-race diagnostic control. CAN is serialized and does not claim a concurrent race campaign. Selected core objects were inspected with nm/size; hosted linkage is not a strict-freestanding closure claim.

Use the corresponding `tools/run_*_followthrough.py --report reviews/v5/expansion/legacy_reactor/<new-name>.json`, strict `check_*_followthrough.py` and `check_*_exports.py` producers. Never overwrite an earlier attempt to conceal failure. These are author reference observations, not non-author review, learner completion, physical validation or release acceptance. Full repository regeneration and integrated campaigns remain pending during parallel authoring.
