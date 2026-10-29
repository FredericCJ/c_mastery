# Assessor: T39 producer to T41 consumer

Keep this file out of learner exports. Grade original T41.02/03 criterion text,
not just one aggregate `NB_PASS` marker. The table in the learner contract gives
the changed inputs; the source-bound campaign names the actual executions.

## 02a

The software route does not locate a physical oscillator, cable or magnetics.
Use the exact inspected UM1974 revisions in the frozen PHY source register and
their page-specific route comparison. The target code configures PA1/2/7,
PB13, PC1/4/5, PG11/13 AF11; board assembly/bridges and the reference source
remain separate premises. An otherwise convincing screenshot with no revision
or measurement location fails. Accept a different route only with authoritative
revision-specific derivation and an explicitly changed port implementation.

## 02b

`struct nb_board` uses actual EpPhy and F439 MDIO functions, not a ready boolean stub.
The composition model routes the emitted selected addresses to the original
EpModel/EnModel, checks pin configuration/base registers and rejects frozen
counter, stuck MDIO, elapsed PHY deadline and stuck DMA reset. Logical failure
does not close the debug clock or recycle a possibly active engine.

In `stop_ownership`, a copied payload has both a flight and a DMA ticket.
Link-down closes the TCB and clears the flight while preserving DMA. Stopped
process bits plus no held RX lease permit release; merely issuing stop does
not. A new PHY incarnation and ring epoch precede a new TCB epoch. An early
`nc_dma_done` in stop request is specifically killed by mutant3.

## 02c

Imported actual PHY mode tests distinguish10/full from100/full and preserve
the50MHz RMII requirement. `NB_BOARD_PASS` is explicitly a software model
result. Require the bench package's board revision, scope input loading,
clock/cable locations, raw capture and calibration before physical completion.
Neither native code, QEMU nor MMIO-object disassembly supplies those observations.

## 03a

12384 is a structural buffer/descriptor budget, not the total application RAM.
Require linked `.eth` address/size, entire interval reachability, all other
state and stack. The negative CCM link must fail with `EN_DMA_REACHABILITY`.
The actual `struct ev_visibility` changes the premise to noncoherent cache lines:
publishing dirty data fails; byte31 shares ownership with byte0; returned9 is
invisible until invalidate; stale clean7 loses9. A DSB cannot repair that loss
or make CCM DMA-reachable. These two-line arrays are an explicit semantic
model, not proof about a particular M7 cache instruction sequence.

## 03b

The copied ring reconstructs control fields on every publication. Application
storage can be changed after successful copy, but the internal copy stays live.
Both ACK/DMA orders pass. A DMA error increments a lower-layer error count and
returns storage once while leaving the flight eligible for transport recovery.
Descriptor refusal leaves SND.NXT1001 and no due flight; later actual admission
at5 starts due1005. A publish poll-write failure occurs after OWN and must
retain the ticket until stop completion. Mutants1/2/3 isolate these edges.

Held-RX wrap tests verify the same pointer's bytes remain unchanged at ring
exhaustion. Returning while stopping clears the CPU lease without setting OWN.
Duplicate return remains stale. The local stop clock crosses4294967295 without
truncation; a much later call saturates its100ms deadline and fails closed.

## 03c

The ring exposes60 bytes from retained-FCS FL64 and1514 fromFL1518. It has
already normalized the length before `struct nb_net`: mutant5 subtracts again and
breaks the handshake. Actual changed `ev_span` takes validated stored bytes:
CSTF1 type0800 permits60 unchanged; length46 and0600 still subtract4. A learner
must not fabricate how changed hardware RDES0 reports FL. Upper parse rejection
still releases a valid descriptor; mutant4 leaks it. Suspended RX and fatal
status are separate ring observations; fatal starts stop rather than promising
safe reuse. Reassembly expiry must precede input at the exact absolute deadline
or the saved first header disappears; mutant6 loses the ICMP11/1 feedback.

## Integration arguments and limits

`nc_receive` and queue consumption can invalidate a prepared packet. Therefore
the adapter deliberately drops incoming TCP and refuses application mutation
while ARP retains its candidate. Mutant7 permits mutation and exposes the stale
ticket. A valid alternative can retain/revalidate a richer transaction, but it
must prove sequence/timer/copy coherence and bound all queues. Silently retrying
an invalid candidate or holding ARP forever is not an alternative proof.

The normal adapter does not change the controller into a congestion experiment:
the separately required NrControl simulator supplies that competency. Each
reference must be identified as assessor work until actual learner artifacts
are substituted and assessed. Capture models exclude FCS electrical generation,
PHY analog behavior, asynchronous bus faults and actual cache timing. Preserve
physical/reviewer/independent/delayed dimensions separately from author results.
