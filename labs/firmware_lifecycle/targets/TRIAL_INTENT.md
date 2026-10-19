# Durable trial intent and one live attempt

This required continuation of `PR-T59-04`, `PR-T59-05` and `G-B06-T59-TARGET` uses the B06 shared exercise contract, `BOOT_STORAGE.md`, `INACTIVE_INSTALL.md`, `FLASH_MAINTENANCE.md` and the exact `trial_intent.h` interface. The active edition remains C17. This is an author implementation and assessment instrument; passing it is neither independent learner assessment nor physical board qualification.

## Prerequisites and environment

Bring the authenticated `BSSelection` from `bs_select`, both current-policy installed `FBImage` results, exclusive bank1 ownership, fixed unswapped H745xI mapping, disabled caches/MPU under the selected cold profile, and the bank2 execution placement in `inactive_install.ld`. Supply callbacks whose context and storage remain live through terminal completion; no callback reentrancy, concurrent policy/image change, interrupt/DMA/peer flash access or released CPU2 is allowed. `FTWriter` starts all-zero and cannot be copied while active. Inputs, control and outputs are distinct live objects. Host signing is a synthetic fixture only; target links the explicitly permitted FW-BOOT verifier/hash provider, never the signer. Ed25519 internals remain a separate mechanism.

## Problem and mechanism

Implement `ft_intent`, `ft_attempt`, `ft_probe`, `ft_begin` and `ft_poll` from their supplied stubs. Reserve bank1 sector5 (`0x080A0000..0x080BFFFF`) only for trial metadata. The installed inactive pair must have equal generation strictly above the committed generation, the correct core numbers, different bases from each active image and nondecreasing security versions. A parent sequence of zero or `UINT32_MAX` cannot start a trial. An intent is not a committed selection and CRC is not authentication.

The exact little-endian intent is128bytes: magic `0x31495446` at0, parent sequence4, trial sequence=parent+1 at8, generation12, two32byte authenticated envelope digests16/48, vectors80/84, versions88/92; commit word at96 contains `0x31435446`, sequence100, generation104, complement(sequence)108, IEEE reflected CRC32 of bytes0..95 at112 and zero116..127. CRC starts/ends XOR`UINT32_MAX` and uses polynomial`0xEDB88320`. The separate32byte attempt at128 contains `0x31415446`, sequence4, generation8, complement(sequence)12, same CRC of bytes0..15 at16, zeros20..31.

`ft_begin` first stages/validates these bytes, then erases exactly sector5. Restore original qualified PSIZE and lock state only after hardware completion; fail on controller errors. Refresh then verify the entire erased128KiB sector, at most32word reads per poll. Program five256-bit words once each: three body words, intent commit word, then attempt word. Only after the last hardware operation, fresh exact readback of all160bytes and final status check may `ft_poll` return `FH_DONE`. A pending poll budget expiry is not cancellation and does not release ownership. `FH_ERROR` is terminal for this object; a controller error requires separately qualified recovery before another operation.

A live peer calls `ft_probe` through the guarded `EGRead` interface, checks exact parent sequence and complete attempt, then independently re-admits both installed images under current key/floor policy and matches every stored digest/vector/version/generation. It publishes its `BSSelection` only after all checks. It must also match the separately published live mailbox via `st_peer_matches` before handoff. This function is forbidden as a cold-reset trial selector.

## Reasoning and reset policy

Explain why intent completion, attempt completion, live launch, application health and joint committed record are five different facts. A reset always invokes `bs_select` on the committed journals and ignores sector5, including a complete attempt whose completion was never acknowledged. A maintenance retry erases sector5 again; it never interprets an all-ones readback as proof that a previous attempt word is virgin. Partial programming can affect ECC even when visible data looks erased. The once-per-erase write rule is established by uninterrupted operation history, not a magic byte pattern after reset.

State the ownership proof for both old image sectors and both journals: this writer has no permitted store into them. Explain why all cold boots before a separately qualified health commit select the previous pair. A CRC collision, forged metadata or readable flash is insufficient to bypass current signed-image admission; trusted mailbox and provisioning are still authority preconditions. Explain why a higher sequence alone is not permission to boot a pair.

## Deliverables

- D1: exact encoding table and state/ownership diagram, including failure before and after each irreversible write.
- D2: original implementations and a link/map audit proving all writer code/constants reside in bank2, no unexpected helpers/signers, static stack obligations and the bound of32erase-verification reads per poll.
- D3: retained traces for every one of40individual32-bit program-store cuts, representative first/middle/final erase cuts, every40metadata read failure and336signed-image read failures, malformed fields, policy changes, controller failure and changed-parent refusal.
- D4: cold-reset and live-peer comparison, plus a changed-condition design in which trial retries are required. Specify an actual new durable format/erase policy; do not reprogram a suspect all-ones word.

## Verification and rubric

Run `tools/run_b06_trial_intent.py` under the declared WSL toolchain. Public fixtures use real signatures over synthetic images and actual production admission/selector functions. A reference run must satisfy every `FT_CHECK`; a learner stub and separate parent-check/attempt-check mutants must fail a semantic assertion. Under each injected reset, restore controller reset state before cold selection and require sequence1; under an intact live trial require sequence2 and both generation2 images. Fail if any rejected `ft_probe` changes output, any write touches the active pair/journals, any target link retains a signer/helper, or any read occurs while the model controller is busy.

Assess `T59.04` trial state using D1/D4: level0 conflates attempted with committed; level1 describes reset only after success; level2 proves the specified reset cases and current-policy rejection; level3 derives the changed retry design and identifies its remaining power-loss assumptions. Assess `T59.05` torn records with D2/D3: level0 trusts CRC as authorization; level1 passes intact encoding; level2 preserves old selection at every tested cut and rejects incomplete intent/attempt; level3 explains which arbitrary hardware faults the digital cut model does not cover. The assessor must require independent explanations and withheld changed-parent/current-policy cases, not accept a copied transcript as mastery.

## Hints

1. Decode with byte operations; do not overlay a packed C structure.
2. Separate erase verification, per-word programming and fresh final verification states.
3. An attempt may be durable even when the caller saw no success. Ask what the next reset is allowed to do before choosing a retry policy.

## Variants and delayed assessment

Keep the actual `PR-T59-04-transfer` and `PR-T59-05-transfer` identities and seven-day delay from `LAB-LC-04B`/`LAB-LC-05B`: the former requires two-core health with one peer failing after the other's health report; the latter removes atomic markers and proves old/new record survival across torn units. `TRIAL_HEALTH.md` composes those obligations with this writer. Additional variants independently redesign retry admission with unknown ECC state, or diagnose a newly revoked key between maintenance sessions. They supplement the original tasks. Another changed condition enables caches; provide exact freshness/execution constraints before implementation. These are authored obligations, not observed learner results.

## Security and limitations

Only the original host controller model, disposable CPU emulators and dedicated teaching board are permitted. Synthetic image/key material has no production authority. The harmless witness is selecting the wrong synthetic generation or accepting an absent attempt; no production credentials, option-byte changes or flashing are authorized by this task. A CRC/sanitizer finding is not an arbitrary-code-execution claim. Host cuts are a digital program/erase model, not physical brownout/ECC measurements. The linked target probe only prepares/probes a durable attempt; maintenance entry, application-health reporting, quiescence, launch and commit composition are separate required work. Exact silicon errata and physical observations remain pending.
