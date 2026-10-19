# PR08 implementation audit

Inspected original PR08, all criteria/V1 and complete T40 L1–L7 fields before
composing the frozen producers. Earlier one-request helper lacked reverse token,
four real byte owners, two independently arbitrated links and exact physical
separation. The new three-node control now adds those mechanisms and actual
quiescent transport reset. It retains the original wire for profile1 and explicitly
routes the registered0x84 changed-incarnation profile separately.

Initial exploratory compilation exposed missing unused channel symbols when the
selected function-section closure flags were omitted; the explicit final closure
uses gc-sections and audits the retained code. An initial disconnect schedule
blocked a preceding credit probe, so it did not demonstrate lost final ACK; the
corrected injection starts only after B's actual application event. No exploratory
failure was reported as a successful source-bound campaign.

The first full campaign passed all six native profiles and the first Cortex-M4
model, then the real-port compile rejected an unnecessarily nested aggregate
initializer under `-Werror=missing-braces`. Its 181-command receipt and exact
board source are retained in `attempt01-board-initializer.json` and `.c`.
The corrected universal zero initializer changes no state-machine behavior;
the complete campaign is rerun with the corrected source identity.

Current evidence must keep physical captures absent. No new global acceptance,
review independence or learner mastery is inferred from author outputs.
