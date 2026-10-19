# Private T21.07–09 discriminator map

| Objective | Required result and reason |
|---|---|
|07.a|Externalcapacity4 is always INVALID; assertion-disabled internal checks disappear only under the selected assertion policy, while explicit production contract branches still run. Sanitizers are not those branches.|
|07.b|Return performs modeledcleanup1; panic exits73,cleanup0; safe simulation setsoutputs0,cleanup0. Broken sink yields one fallback record without recursively retrying. No physical safety credit.|
|07.c|The deliberately flawed assertion increments1/debug and0/NDEBUG. The repair always increments1 after admission. Disabled optional macro arguments vanish from preprocessed client and have zero side effects.|
|08.a|Golden bytes are1,3,1,16,3,4,255,255,1,4,9,2. Check240 field combinations and65536 width8 pairs. Values0,100,200 show modular comparisons cannot define historical total order.|
|08.b|A..E producesCDE/dropped2. Stalled sink keeps the head; rate admission and transport stall counters differ. Protect-high rejects full-queue newhigh and any eviction when a queuedhigh exists; tokens remain.|
|08.c|The sink calls ae_report while busy, so only one sink frame exists. Nested and outer failure fill two records, then loss increments twice per failed outer attempt. Missing fallback means explicit loss, not silent success.|
|09.a|The same consumer reads two private layouts through a16-byte frame and observes publicTIMEOUT/op9/cause104. Every shorter length, excess length, changed version, reserved byte and operation mismatch rejects.|
|09.b|With switchesd,i, payload evaluations ared+2i after lowering threshold. Runtime filtering still requires a compiled integrator. Dedicated buffer symbols must match each switch.|
|09.c|Section GC removes an unreferenced disabled emit function; another TU's function address preserves it and explicit arguments evaluate. No dedicated buffer appears in either disabled build.|

The fallback and disabled-evaluation mutants must compile and fail the intended
semantic check. Learner stubs compile and fail. Assert-abort and the intentionally
flawed side-effect demonstration are separate diagnostic observations, not semantic
mutant successes. The absence of a buffer symbol alone is insufficient: inspect
preprocessing, runtime evaluations and sections too.

Valid alternatives may encode a different specified wire version or choose a
different priority policy, but must preserve the original requested behavior in a
compatible adapter and expose the changed policy rather than silently weakening
tests. Delay changed packets by the original minimum seven days; assess each row
independently. A generic logging implementation does not earn all nine criteria.
