# Counter notifications and callback lifetime: current author validation

Receipt: `reviews/v5/expansion/legacy_timers/current_validation.json`

SHA256: `9f132cfc6b076bbe5b3e1196dd0185b66eca31603d5751f8cbf167ad1839e6d6`

Observed: 110 exact commands, 6 ordinary/changed observations; campaign passed=True. 23 report controls passed. Actual CLI export passed=True; 6 additional standalone export commands where supplied. Original criterion bodies and seven-day delays are preserved by the integration receipt.

Includes actual timerfd/eventfd, earliest-deadline rearm using frozen hs_read_until, native clock-bracket observations,14112 independent calendar cells/profile,200 actual callback races/profile, reduced generation retirement and full-width phase exhaustion. Pilot/current receipts are byte-identical because no campaign inputs changed after the successful run.

Every compiler invocation, result, environment, source identity, raw output, expected rejection and retained artifact is in the JSON receipt. Native GCC/Clang debug/optimized/ASan+UBSan profiles were actually run; reactor/timers additionally run both TSan profiles with a known-race diagnostic control. CAN is serialized and does not claim a concurrent race campaign. Selected core objects were inspected with nm/size; hosted linkage is not a strict-freestanding closure claim.

Use the corresponding `tools/run_*_followthrough.py --report reviews/v5/expansion/legacy_timers/<new-name>.json`, strict `check_*_followthrough.py` and `check_*_exports.py` producers. Never overwrite an earlier attempt to conceal failure. These are author reference observations, not non-author review, learner completion, physical validation or release acceptance. Full repository regeneration and integrated campaigns remain pending during parallel authoring.
