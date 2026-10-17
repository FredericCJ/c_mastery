# PR-CAP-MCU-03 capstone composition assessor analysis

Withheld technical analysis. The learner receives the shared capstone contract, project supplement and public fixtures. Assess the specific integration claims below; passing a retained component gate is not complete capstone acceptance.

<a id="t3109b"></a>
## T31.09.b

Commit-last works only if the validity predicate rejects torn/incomplete candidates and old-slot preservation is guaranteed. CRC and trusted authentication verdict are different predicates; the latter is a model input here, not invented signature code. Handoff also requires stack/vector/interrupt contracts before real execution.

**Independent expected relation:** Cuts0..4 select A; cut5 selects B only when all validity/compatibility/trust predicates and commit hold. An untrusted candidate selects A. The model proves neither actual flash atomicity nor cryptographic verification.

**Reject superficial evidence:** Selecting the highest CRC-valid generation is a secure-update implementation. The learner must show the causal boundary and an independent discriminator; an unrelated compiler/runtime failure is not a semantic negative control.

**Delayed transfer review:** Make commit writes tear or generations wrap; provide an explicit encoding/order policy and counterexamples to naive comparison. Select a new compatible event history and require predictions before execution. Check updated preconditions, resource accounting and retained successful behavior, not recall of the supplied values.

<a id="t3110c"></a>
## T31.10.c

Health requires the selected useful-work obligations within the current epoch, not accumulated historical bits. Detection without containment can restart the same hazardous action. Fault records need bounded nonrecursive capture; physical reset reason and detection latency remain unmeasured until board evidence.

**Independent expected relation:** Feed decisions are false,true,false,true. Acquisition alone cannot keep the watchdog healthy; repeated faults reach SAFE_STOP after the declared recovery attempts and do not reapply an earlier external effect.

**Reject superficial evidence:** Any interrupt activity or reboot is evidence of healthy recovered progress. The learner must show the causal boundary and an independent discriminator; an unrelated compiler/runtime failure is not a semantic negative control.

**Delayed transfer review:** A reset can occur during crash-record commit. Define record validity/sequence and distinguish absent/incomplete capture from no fault. Select a new compatible event history and require predictions before execution. Check updated preconditions, resource accounting and retained successful behavior, not recall of the supplied values.

<a id="t3508c"></a>
## T35.08.c

C encapsulation is a design convention, not same-process access control. MPU configuration must be matched to privilege and bus-master behavior; DMA is not automatically constrained by a CPU MPU. This task is analysis/model work until exact target manuals and board tests exist.

**Independent expected relation:** The dossier states exact enforcement by compiler discipline, runtime checks and hardware; unsupported MPU/DMA enforcement remains open. No protected host memory or real deployed equipment is probed.

**Reject superficial evidence:** A private header or an MPU rule for CPU accesses automatically confines all code and DMA. The learner must show the causal boundary and an independent discriminator; an unrelated compiler/runtime failure is not a semantic negative control.

**Delayed transfer review:** A privileged interrupt handler receives an untrusted request buffer. Define validated copy-in, lifetime and privilege-transition obligations before claiming isolation. Select a new compatible event history and require predictions before execution. Check updated preconditions, resource accounting and retained successful behavior, not recall of the supplied values.

