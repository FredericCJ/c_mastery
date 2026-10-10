"""Maintain original integration briefs and synthetic fixture oracles; no solved C."""
import json
import zlib
from common import ROOT, write, dump

def main():
    folder=ROOT/'fixtures/integration'
    queue={'id':'integration-queue','description':'Representation-independent bounded FIFO contract',
        'input':{'capacity':4,'operations':[['put',11],['put',22],['put',33],['put',44],['put',55],['get'],['get'],['put',66],['put',77]]},
        'expected':{'statuses':['OK','OK','OK','OK','FULL','OK','OK','OK','OK'],'removed':[11,22],'remaining':[33,44,66,77]},
        'verification_method':'Independent Python list model; rejected put preserves state'}
    elf={'id':'integration-binary','description':'Synthetic symbol closure and partial-write trace, not observed machine ELF',
        'input':{'allowed':['write_adapter','exit_adapter','memcpy_local'],'referenced':['write_adapter','exit_adapter','memcpy_local','malloc'],
            'write_request_bytes':9,'write_outcomes':[3,'EINTR',2,4]},
        'expected':{'forbidden':['malloc'],'remaining_before_call':[9,6,6,4],'transferred':9},
        'verification_method':'Set difference and arithmetic trace; actual binary audited separately'}
    wire={'id':'integration-wire','description':'Small framing fixture, raw bytes and independent IEEE CRC oracle',
        'input':{'version':1,'sequence':258,'payload_hex':'0041ff','max_payload':32},
        'expected':{'frame_hex':'','frame_bytes':13,'maximum_frame_bytes':42,'empty_frame_bytes':10},
        'verification_method':'zlib.crc32 over version/sequence/length/payload; byte-count formula'}
    body=bytes([1])+int(258).to_bytes(2,'little')+int(3).to_bytes(2,'little')+bytes.fromhex('0041ff')
    wire['expected']['frame_hex']=(b'\xa5'+body+zlib.crc32(body).to_bytes(4,'little')).hex()
    irq={'id':'integration-irq','description':'Synthetic timestamp and capacity model; no measured interrupt timing',
        'input':{'counter_bits':16,'start':65530,'end':6,'capacity':3,'samples':[100,101,102,103,104]},
        'expected':{'elapsed_ticks':12,'accepted':[100,101,102],'dropped':2},
        'verification_method':'Modulo arithmetic, half-range assumption and bounded-prefix model'}
    pressure={'id':'integration-pressure','description':'Deterministic queue ownership trace with reject-newest policy',
        'input':{'capacity':3,'events':[['offer',1],['offer',2],['offer',3],['offer',4],['take'],['offer',5],['close'],['offer',6],['take'],['take'],['take']]},
        'expected':{'accepted':[1,2,3,5],'rejected':[4,6],'consumed':[1,2,3,5],'remaining':[],'closed':True},
        'verification_method':'Sequential reference ownership ledger, not a concurrency proof'}
    fir={'id':'integration-fir','description':'Exact rational causal FIR and illustrative benchmark samples',
        'input':{'samples':[0,4,8,4,0],'coefficient_numerators':[1,2,1],'denominator':4,'zero_prehistory':True,'runs_ns':[100,105,95,110,90]},
        'expected':{'output':['0','1','4','6','4'],'median_ns':100,'min_ns':90,'max_ns':110},
        'verification_method':'Fraction arithmetic and statistics; synthetic runs are not performance evidence'}
    recovery={'id':'integration-recovery','description':'Synthetic update-slot policy with no bootloader implementation',
        'input':{'slots':[{'name':'A','generation':7,'valid':True},{'name':'B','generation':8,'valid':False}]},
        'expected':{'selected':'A','generation':7,'invalid_ignored':['B']},
        'verification_method':'Select highest generation among valid records; authenticity/failure model separately required'}
    fixtures={'queue':queue,'binary':elf,'wire':wire,'irq':irq,'pressure':pressure,'fir':fir,'recovery':recovery}
    for name,value in fixtures.items(): dump(folder/(name+'.json'),value)
    # Each tuple records distinct new work on already implemented components.
    specs=[
      ('P1','01','Bounded library integration','C',['T14.01','T14.05','T16.02','T18.04'],'queue',
       'Assemble the existing vector/ring and arena/pool behind caller-storage interfaces. Freeze capacity, alignment, ownership and error conventions; do not rewrite the prior algorithms.',
       'Combine two previously tested components in a hosted harness. Fill the queue, reject one extra insertion, remove two values and wrap storage. Allocate aligned records through the declared allocator interface and explain their lifetime across an arena reset.',
       'Submit public headers, state-transition traces, invariants and a lifetime diagram. The queue fixture must match without depending on internal index layout; rejected operations preserve state. Show a second backing store with the same contract.'),
      ('P1','02','Exhaustion and dependency closure','C',['T13.01','T16.14','T21.04'],'queue',
       'Audit the integrated library under fixed storage and injected allocation failure. No external C-library call may remain in the component unless the contract explicitly permits a compiler support helper.',
       'Inject failure at each allocation ordinal reached by the selected workflow. Verify cleanup and unchanged ownership. Cross-compile the core, inspect undefined symbols and trace callbacks for allocation or blocking. Tighten capacity by half and rerun.',
       'Submit failure-index results, dependency allowlist, object audit and maximum storage calculation. Separate the hosted harness from the audited core. Any opaque transitive behavior must be disclosed rather than certified allocation-free.'),
      ('P2','01','Custom entry meets the portable core','D',['T09.01','T10.01','T11.01'],'binary',
       'Integrate a previously written x86-64 Linux entry/exit adapter with one portable component. Declare stack alignment, entry arguments, selected syscalls and every linked helper.',
       'Link without the usual startup objects using explicit driver controls. Inspect entry address, load segments, symbols and relocation requirements. Write a bounded diagnostic through your syscall adapter and terminate by the declared exit syscall.',
       'Submit link command, map, ELF observations and startup responsibility diagram. The synthetic forbidden-symbol case must reject malloc. The real image audit must explain resolved helpers, not only show an empty undefined-symbol list.'),
      ('P2','02','Short transfer and syscall dossier','D',['T10.07','T23.04','T21.02'],'binary',
       'Complete the custom-entry executable\'s transfer/error contract, including short writes, interruption, zero progress and permanent errors. Keep ABI-specific behavior behind the adapter.',
       'Feed the supplied deterministic write outcomes through a test adapter, then run the executable with actual Linux syscall tracing. Compare adapter and kernel evidence. Remove one optional helper and document the affected build/dependency proof.',
       'Submit the nine-byte transfer trace and syscall/dependency inventory. EINTR retries do not advance the pointer; positive returns do. Zero progress must not cause an unbounded busy loop. Distinguish raw negative syscall results from libc errno conventions.'),
      ('P3','01','One wire format for both tracks','C',['T04.06','T15.12','T18.01'],'wire',
       'Implement the shared educational wire contract in the integration guide using your existing byte primitives. Storage is caller-provided, payload length is at most 32 and no C structure overlays wire bytes.',
       'Write encoder and whole-frame validator, checking every capacity calculation before access. Compare bytes with the independent CRC fixture. Include empty payload, maximum payload, embedded zeroes and sequence wrap. Document application sequence policy separately.',
       'Submit byte-level diagram, encoder/decoder tests and bounds argument. The supplied frame is exactly thirteen bytes and the maximum is forty-two. CRC rejection emits no message; CRC is not authentication.'),
      ('P3','02','Streaming and malformed-input campaign','C',['T15.13','T21.05','T32.06'],'wire',
       'Adapt the frame decoder to arbitrary chunk boundaries and the stated resynchronization rule. It may retain at most one bounded candidate and declared state; incomplete input never fabricates a frame.',
       'Test every split position of the known frame, then concatenate valid frames with noise, a bad CRC, unsupported version and oversized length. Inject one-byte-at-a-time input and an output consumer that temporarily refuses a message. Minimize any failing stream.',
       'Submit state machine, memory bound, split campaign and rejection/recovery trace. Define whether completed messages are retained or backpressured. Add a changed chunk schedule unavailable during first implementation and justify identical decoded output.'),
      ('P4','01','Acquisition to bounded frames','E',['T27.08','T28.05','T29.06'],'irq',
       'Integrate explained F439 startup, a register-level acquisition/serial adapter and a bounded foreground/ISR queue. Use P3 for output framing; define drop and ownership policy.',
       'Begin with a host event model, then build the actual image and test a documented board configuration. Use polling first, then a supported interrupt or DMA path. Record where each buffer resides and why DMA can reach it; CCM is unsuitable for the selected DMA path.',
       'Submit startup/map audit, driver contract, synthetic overflow trace and actual-board functional evidence. No ISR allocates or blocks. A host or MPS2 result cannot stand in for STM32 register behavior.'),
      ('P4','02','Timing and recovery boundaries','E',['T29.09','T30.05','T31.10'],'irq',
       'Audit wraparound, loss, fault recovery and measured latency for the firmware pipeline. Keep analytical bounds distinct from observed maxima.',
       'Apply the synthetic sixteen-bit wrap case and a burst exceeding queue capacity. On the board inject a declared peripheral timeout and verify recovery. Measure entry-to-service and end-to-end latency with appropriate instrumentation, retaining clock assumptions and setup.',
       'Submit loss counters, timer arithmetic proof within the half-range assumption, recovery trace and latency distribution. Rebuild with a smaller buffer. Report hardware evidence as pending if the board is unavailable; never manufacture a physical timing result.'),
      ('P5','01','Concurrent ownership and backpressure','A',['T24.08','T25.02','T18.04'],'pressure',
       'Integrate a Linux reader, bounded queue and processing worker using the shared decoder. Each message has one declared owner. Overload uses reject-newest or an explicitly justified alternative.',
       'First reproduce the deterministic offer/take/close trace, then implement synchronization around the queue predicate. Use predicate loops for condition waits. Simulate a slow consumer and measure the configured capacity ceiling without silently allocating extra backlog.',
       'Submit ownership table, synchronization argument, capacity ledger and concurrent tests. The supplied accepted/rejected/consumed lists must reconcile exactly. A stress pass supplements rather than replaces race and lifetime reasoning.'),
      ('P5','02','Shutdown under load','A',['T24.11','T25.04','T21.06'],'pressure',
       'Implement graceful close with a declared drain policy and bounded cancellation/error behavior. Closing prohibits new offers, wakes waiting participants and preserves ownership of queued messages.',
       'Exercise close before start, close while empty/full, a blocked producer, partial network input and a failed consumer. Separate readiness handling from message ownership. Collect actual resource and syscall evidence under repeated slow-client runs.',
       'Submit shutdown state diagram, timeout/error tests, leak/dependency evidence and load measurements. All accepted work is drained or accounted for by the declared abort policy. A changed capacity/concurrency strategy must preserve the portable core.'),
      ('P6','01','Numerical contract before optimization','C',['T17.02','T17.06','T34.01'],'fir',
       'Integrate the existing causal FIR or selected computational kernel with a scalar reference and explicit numerical contract. No equivalent library routine may implement the studied core.',
       'Reproduce exact rational fixture outputs with zero prehistory. Add cancelling, large/small and tail-length cases appropriate to the chosen floating/fixed representation. State absolute and relative tolerance, overflow policy and error measure before benchmarking.',
       'Submit scalar implementation, independent oracle rationale, error observations and operation/storage model. Synthetic nanosecond samples teach summary calculation only; they are not measured speed claims.'),
      ('P6','02','Vectorization and reproducible measurement','C',['T34.06','T34.07','T34.09'],'fir',
       'Compare scalar, compiler-vectorized and an explicit SIMD variant on supported real x86 hardware. Preserve the P6 numerical contract, scalar fallback and correct tail handling.',
       'Inspect optimization reports/disassembly and verify actual ISA/OS support before dispatch. Benchmark representative sizes with warm/cold distinctions, retained results and repeated runs. Explain differences in work, memory traffic and measurement overhead.',
       'Submit correctness comparisons, generated-code evidence, raw run data and variability summary. A slower optimized variant is an acceptable learning result when explained. Fast-math changes require a revised contract and cannot silently relax the gate.'),
      ('CAP-LINUX','01','Linux architecture and threat model','A',['T20.01','T22.01','T35.01'],'wire',
       'Specify a bounded telemetry service that reuses P3 and P5, with input source, message processing, output sink and operational limits. Select architecture from at least two alternatives.',
       'Draw boundaries and ownership transitions, enumerate malformed input, overload, external failure and shutdown threats. Freeze memory, message and descriptor ceilings. Map reusable evidence to the contract and identify the remaining integration tests.',
       'Submit architecture decision record, threat/failure model, dependency profile and traceability plan. This milestone adds design integration; existing parser and queue implementation hours are not charged again.'),
      ('CAP-LINUX','02','Service assembly and packaging','A',['T22.05','T23.04','T24.08'],'pressure',
       'Assemble the service from reviewed components with adapters, configuration and a reproducible build. Keep portable headers free of Linux-specific dependencies.',
       'Run fixture streams through real input and output adapters. Build the portable core separately for firmware. Package public headers and consume the library from a second small build tree. Record compiler/libc versions and final dependency closure.',
       'Submit executable, package/consumer evidence, end-to-end results and resource ledger. Input/output failures follow the stated policy, not an assertion or silent data loss. Account for every accepted message.'),
      ('CAP-LINUX','03','Fault and overload campaign','A',['T32.07','T35.06','T21.04'],'pressure',
       'Challenge the assembled service with allocation, I/O, malformed input and slow-consumer failures while checking capacity and ownership invariants.',
       'Inject failures at component boundaries, fuzz the protocol core with bounded campaigns and exercise shutdown races. Preserve minimized regressions. Inspect hardening/dependencies and document which controls actually exist in the chosen build.',
       'Submit reproducible campaign commands, findings and fixes, memory/dependency evidence and load data. Test coverage is supporting evidence; it cannot compensate for unexplained lifetime or race defects.'),
      ('CAP-LINUX','04','Linux defense and transfer','A',['T20.10','T33.10','T35.12'],'wire',
       'Defend the completed service against the common mastery gate, then change one substantial boundary without rewriting the portable core.',
       'Choose a changed allocator, tighter capacity, different backend or concurrency strategy. Predict affected invariants and evidence before modifying code. Repeat relevant gates and perform delayed retrieval of a critical reasoning task.',
       'Submit complete dossier, recorded defense, perturbation comparison and unresolved limitations. Acceptance requires every mandatory dimension at level two and transfer evidence at level three; performance cannot offset correctness failure.'),
      ('CAP-MCU','01','Firmware architecture and resource contract','E',['T20.01','T30.07','T35.01'],'irq',
       'Specify a bounded F439 acquisition/command firmware image sharing the Linux protocol core. Define boot, normal work, overload, fault and recovery states.',
       'Compare superloop/state-machine alternatives and justify the selected contexts. Freeze memory/stack, peripheral timeout, queue and timing contracts. Record board/device/manual revisions and distinguish host/model/physical evidence requirements.',
       'Submit architecture decision record, context/ownership diagram, resource ceilings and traceability plan. Existing P3/P4 work is reused; this milestone adds integration decisions and missing tests.'),
      ('CAP-MCU','02','Image assembly and target acceptance','E',['T27.10','T28.11','T31.08'],'wire',
       'Assemble and deploy the firmware image with its own documented startup/linker/runtime closure, acquisition adapter and shared protocol core.',
       'Build the core on host and cross target, inspect the image/map and flash the recorded board. Exercise valid, empty, maximum and malformed frames through the actual interface. Capture reset-to-main, data/bss initialization and peripheral acceptance observations.',
       'Submit image identity, build/deployment record, startup audit, host regressions and actual-board functional results. Keep DMA placement and stack constraints visible. A simulator-only run leaves physical acceptance pending.'),
      ('CAP-MCU','03','Fault containment and update policy','E',['T31.09','T31.10','T35.08'],'recovery',
       'Add documented fault records, watchdog/progress policy and a defensible update/boot handoff design. Implement the safe fault/recovery slice within the existing application; a production bootloader is not required.',
       'Use the synthetic slot fixture to explain invalid-image rejection. Inject a selected recoverable peripheral failure and a controlled software fault on the board. Record persistent-state validity, reset reason, ownership cleanup and recovery limits. Discuss authenticity separately from CRC validity.',
       'Submit fault decoding, reset/recovery traces, watchdog rationale and update threat model. Power-loss and authenticity claims need evidence beyond the toy slot policy. Never describe selecting a valid record as secure-update certification.'),
      ('CAP-MCU','04','Firmware defense and transfer','E',['T30.10','T33.10','T35.12'],'irq',
       'Defend the full firmware dossier and shared-core portability, then perturb capacity, backend, clock configuration or a permitted runtime dependency.',
       'Predict which timing/resource arguments change before modification. Repeat map/symbol, functional, race/ownership and physical timing evidence as applicable. Rebuild a small critical component after a delay without its old source.',
       'Submit defense, perturbation comparison, shared-core build evidence and explicit pending limits. Accept only when mandatory memory/correctness/failure/dependency gates pass and the required physical evidence exists.')
    ]
    for family,num,title,profile,topics,fixture,contract,steps,accept in specs:
        pid=f'PR-{family}-{num}'
        meta={'id':pid,'title':title,'module':'INTEGRATION','topics':topics,'status':'core','hours':8,'profile':profile,'fixtures':[f'fixtures/integration/{fixture}.json']}
        body=f'''## Contract

{contract} This is an integration milestone: link the prior component dossier and record only the new work. Apply the profile boundary in {{ref}}`guide-profiles` and the common review in {{ref}}`guide-assessment`.

## Inputs and expected results

Use `fixtures/integration/{fixture}.json`. It contains an explicit input and independently recomputed expected model result. Predict the result before inspecting the answer. Synthetic data validates the stated model only; observations from actual binaries, concurrent runs or hardware must be recorded separately. Include one new normal case, one boundary and one failure case specific to this contract.

## Steps

{steps}

## Failure cases

Reject out-of-contract sizes before access. Preserve ownership and invariants when a backend fails, an operation cannot progress or storage is exhausted. Select applicable failures from the contract above and demonstrate each response. Retain the smallest reproducer and a regression check; unexplained termination or a fabricated successful observation fails the gate.

## Acceptance and evidence

{accept} Link evidence to the covered topic objectives and state what each artifact establishes. Project hours include implementation and verification; review and later retrieval have separate budgets.

## Hints and recovery

Start from the smallest deterministic fixture and write a state/ownership table. If actual behavior disagrees, separate the model, adapter and environment before editing the core. Consult the linked module's bounded readings and reduce the failure to one boundary transition. Do not resolve an oracle disagreement by changing the expected result without a contract argument.

## Transfer challenge

Change one declared capacity, backend or environment assumption. Predict the surviving invariants and regenerate the affected evidence. Explain the cost of the change and preserve the previous regression. No complete C solution is supplied: the implementation and its defense are learner work.
'''
        write(ROOT/'curriculum/projects'/f'{pid}.md','```json\n'+json.dumps(meta,indent=2)+'\n```\n\n'+body)
    dump(ROOT/'curriculum/integration_families.json',{'families':[{'id':f,'milestones':[f'PR-{f}-{n:02}' for n in range(1,5 if f.startswith('CAP') else 3)],'additional_workload':{'reading':2,'assessment':2,'consolidation':2}} for f in ['P1','P2','P3','P4','P5','P6','CAP-LINUX','CAP-MCU']]})
    print('Created 20 integration milestones and 7 independent model fixtures.')

if __name__=='__main__': main()
