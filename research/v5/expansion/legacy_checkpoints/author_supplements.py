"""Render individually authored checkpoint composition assessments additively."""
from pathlib import Path
import hashlib
import json
import re
import sys

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(ROOT/'tools'))
sys.path.insert(0, str(ROOT))
from cards import rows
from inputs import write
from common import load
from research.v5.expansion.legacy_completion.support import attach as attach_successor_support
from research.v5.expansion.legacy_checkpoint_memory.author_support import attach as attach_memory_followthrough
from research.v5.expansion.legacy_checkpoint_entry.author_support import attach as attach_entry_followthrough
from research.v5.expansion.legacy_checkpoint_writer.author_support import attach as attach_writer_followthrough
from research.v5.expansion.legacy_checkpoint_firmware.author_support import attach as attach_firmware_followthrough
from research.v5.expansion.legacy_checkpoint_protocol.author_support import attach as attach_protocol_followthrough
from research.v5.expansion.legacy_numeric_followthrough.author_support import attach as attach_numeric_followthrough
from research.v5.expansion.legacy_checkpoint_linux.author_support import attach as attach_linux_followthrough

REGISTERS = ['research/v5/expansion/legacy_checkpoints/resources.json',
             'research/v5/expansion/legacy_systems/resources.json',
             'research/v5/expansion/legacy_runtime/resources.json', 'research/v5/b06_resources.json']
SOURCES = {
 'P1': ['R-LCP-C11', 'R-LSYS-GCC-LINK'],
 'P2': ['R-LSYS-ELF', 'R-LSYS-GCC-LINK', 'R-LSYS-GCC-ASM', 'R-LSYS-WRITE'],
 'P3': ['R-LCP-C11', 'R-LCP-ZLIB'],
 'P4': ['R-LRUN-CM4', 'R-LRUN-RM21', 'R-LRUN-C11-ATOMICS'],
 'P5': ['R-B06-EPOLL', 'R-LRUN-THREAD', 'R-LRUN-COND', 'R-LRUN-C11-ATOMICS', 'R-LCP-C11'],
 'P6': ['R-LCP-C11', 'R-B06-GCCMATH']}
LIMIT = {
 'P1': 'integration.p1 tests accepted-prefix decimal-record/queue handoff; it does not implement or assess the required allocator/backing-store composition.',
 'P2': 'integration.p2 tests a retained active-byte writer adapter; it does not establish real entry ABI, ELF mapping, syscall translation or generated-code closure.',
 'P3': 'integration.p3 is a length-byte/decimal-record parser with terminal failure, not the shared A5/version/sequence/length<=32/CRC protocol and resynchronization required here.',
 'P4': 'integration.p4 is a serialized DMA generation/quiescence-to-queue adapter. It does not establish F439 reset, IRQ concurrency, actual UART/DMA behavior or physical timing.',
 'P5': 'integration.p5 is a synchronous cancellation/completion/discard anchor; it does not establish the concurrent Linux reader/decoder/worker or load/shutdown campaign.',
 'P6': 'integration.p6 uses a fixed integer filter and synthetic measurement capture; it does not establish the required rational FIR, numerical equivalence, SIMD dispatch or actual speedup.'}
TARGET = {
 'P1': 'Caller-storage C17 core, hosted verification harness and explicitly identified cross-compiled image for dependency audit.',
 'P2': 'Original x86-64 Linux custom-entry executable in an ordinary isolated local process; scripted adapters are separate host models.',
 'P3': 'Original bounded C17 byte-protocol core in a host harness; portable target runs record their actual ABI/data model.',
 'P4': 'Dedicated STM32F439 teaching board or explicitly identified emulator. Host event models are separate and cannot discharge physical/board obligations. This authoring work performs no flashing.',
 'P5': 'Original Linux pthread/epoll service using bounded local synthetic inputs; no external endpoints or credentials. Deterministic models and actual threaded runs are separate.',
 'P6': 'Portable scalar C17 core and separately identified x86 SIMD variant where ISA/OS support is inspected and available; host rational oracle is not a target dependency.'}
HINTS = {
 'P1-01': ['Draw separate storage, valid-record, queue and callback ownership rows before selecting an allocator representation.', 'For each handoff mark the last permitted use of the old view. For arithmetic reserve the header, then padding, then payload; refusal preserves the last committed offset.'],
 'P1-02': ['List the resources owned after each successful acquisition, including callbacks that may still be running.', 'Inject the first failure after every success prefix. The safe cleanup order is a dependency order, and link closure includes resolved callbacks and compiler-emitted helpers.'],
 'P2-01': ['Distinguish the loaded memory map, ELF entry and C function-entry state.', 'Draw the initial stack and first call separately. Audit actual program headers and symbol providers; driver switches do not supply missing runtime mechanisms.'],
 'P2-02': ['Keep accepted prefix, pending suffix and retained active transfer state in separate columns.', 'A retry does not move the cursor. Map backend result conventions before aggregation, and preserve the first cause if cleanup or diagnostics also fail.'],
 'P3-01': ['Write each byte offset and CRC extent before writing a decoder.', 'Separate representable length, allowed length, available input and output capacity. A checksum carries no secret and cannot identify an authorized sender.'],
 'P3-02': ['Mark validation and downstream delivery as different commit points.', 'For recovery resume after the rejected sync, not after its declared length. Count retained bytes and rescans; pause input if the completed output still belongs to the parser.'],
 'P4-01': ['Separate core, peripheral and board responsibilities, and record who owns each buffer at each event.', 'Payload publication precedes the SPSC head update. TXE, DMA completion and TC describe different boundaries; no host timestamp establishes the pin-level completion.'],
 'P4-02': ['Draw acquisition, interrupt, queue, DMA and physical-completion times on one event line.', 'Timeout changes software policy but does not prove hardware quiescence. Preserve generation and buffer ownership until the documented stop/acknowledgement boundary.'],
 'P5-01': ['Count all live message representations, including parser and worker ownership outside the queue.', 'Validate descriptor/slot generation before dereferencing context; copy or transfer payload ownership before resetting the parser buffer.'],
 'P5-02': ['Write terminal dispositions for every accepted message before choosing shutdown transitions.', 'Waiters loop on predicates under the mutex; cancellation request and acknowledged completion are separate. Duplicate callbacks must be rejected before released context is touched.'],
 'P6-01': ['Write the scalar recurrence, history order and allowed numerical error before timing.', 'Compare final state as well as output across chunk splits. Extent refusal can be transactional even when a later arithmetic error requires a separate prefix/rollback policy.'],
 'P6-02': ['Seal the correctness contract and runtime ISA prerequisites before reading performance results.', 'An unaligned load still needs a valid extent. Inspect the actual loop and consume its output; repeat paired measurements and preserve noise and correctness failures.']}
FAMILIES = {
 'P1-01': ['ABCIJ','BCFGJ','ABCFJ','BCDGJ','BCDFJ'],
 'P1-02': ['ADFIJ','BCFGJ','BHFJ','BCDGJ'],
 'P2-01': ['ADFJ','ABDIJ','ADFIJ'],
 'P2-02': ['BCFGJ','BCDFGJ','ABDIJ','BCDGJ'],
 'P3-01': ['ABCIJ','ABCFJ','ABCEFJ','ACFGJ'],
 'P3-02': ['BCFGJ','BCDFHJ','BCFGJ','BCDFJ','CDFJ'],
 'P4-01': ['BDIJ','BCGIJ','BCDFJ','FHIJ'],
 'P4-02': ['BCDFJ','ABFHJ','FHIJ','BCDGJ'],
 'P5-01': ['BCDFJ','BCFGHJ','ABFIJ','BCDGJ'],
 'P5-02': ['BCDGJ','BCDFGJ','BCFGJ','BCDFGJ'],
 'P6-01': ['ACFIJ','ABCFJ','BCFGJ','AFHJ'],
 'P6-02': ['ACDFIJ','BCFIJ','DFHJ','FHIJ']}
EXTRA = {'P4-01':['PR-P3-02'], 'P4-02':['PR-P3-02'],
         'P5-01':['PR-P1-02','PR-P3-02'], 'P5-02':['PR-P1-02','PR-P3-02']}


def read(path):
    return json.loads(path.read_text(encoding='utf-8'))


def pin(path, scope):
    return dict(path=path.relative_to(ROOT).as_posix(), sha256=hashlib.sha256(path.read_bytes()).hexdigest(), passages=[scope])


def security(oid):
    if oid != 'T15.12.c':
        return dict(applicable=False, scope='Correctness, ownership or measurement assessment of original teaching programs with synthetic data. Any flawed code is confined to a bounded local harness. Firmware experiments use dedicated boards/emulators; no flashing, deployed equipment, host policy changes or external service targeting is authorized. Crashes, undefined behavior or sanitizer reports do not establish arbitrary execution.')
    return dict(applicable=True,
        scope='Original P3 byte protocol in a bounded local in-memory teaching harness with synthetic payloads only.',
        target='Learner-written CRC-framed P3 decoder and an intentionally mistaken authorization decision based solely on CRC validity.',
        environment='Unprivileged local process; at most256 synthetic input bytes per run, no network or filesystem targets.',
        attacker_capabilities='Modify payload bytes and recompute the public CRC; no keys, privileges or external access are supplied.',
        isolation='Original code and synthetic values only, bounded run, no credentials, network, production firmware or third-party target.',
        defect='Treating an unkeyed error-detection checksum as sender authentication allows attacker-selected messages to pass that mistaken authorization gate.',
        exposing_transition='Accept the known frame, flip payload00 to01 without CRC update and observe rejection, then recompute CRC and observe acceptance of0141ff by the original framing validator.',
        exploitability_limits='The violation requires the application to authorize based only on checksum validity. Framing acceptance alone is expected behavior, not a flaw in CRC. No execution, privilege or real-system compromise is claimed.',
        success_condition='A synthetic protected-value counter in the deliberately flawed teaching application changes for the recomputed unauthorized frame; no outside state is touched.',
        repair='Keep framing and authorization separate. The corrected target refuses to alter protected state in its unauthenticated mode; ordinary public framing may still accept the changed payload. If authenticated commands are required, add an independently specified keyed authentication and replay policy only after inspecting its primitive contract.',
        regression='Run original/stale-CRC/recomputed-CRC frames through flawed and corrected application gates. The corrected gate never changes protected state without authorization; ordinary framing behavior and valid CRC vectors remain unchanged.',
        residual_limits='Disabling unauthenticated protected commands contains this teaching defect but does not implement authenticated messaging. Key management, replay resistance, cryptographic primitive implementation and side channels remain separate required work for the authenticated transfer variant.')


def main():
    resources = {}
    for path in REGISTERS:
        for resource in read(ROOT/path)['resources']:
            assert resource['id'] not in resources
            resources[resource['id']] = (path, resource)
    tax = load()[0]
    leaves = {o['id']:o.get('text',o.get('title')) for d in tax for t in d['subtopics'] for o in t['objectives']}
    units = read(ROOT/'curriculum/v4/units.json')['units']
    graph = {p['id']:p for p in read(ROOT/'curriculum/project_graph.json')['projects']}
    data = rows()
    audit, trace, dispositions, pending = [], [], [], []
    for pid, cards in data.items():
        short = pid[3:]
        wave = short[:2]
        project = ROOT/'curriculum/projects'/(pid+'.md')
        meta = json.loads(re.search(r'```json\s*\n(.*?)\n```',project.read_text(encoding='utf-8'),re.S)[1])
        pu = [u for u in units if u['project']==pid]
        selected = {r['objective'] for r in cards}
        parent_leaves = {o for o in leaves if o.rsplit('.',1)[0] in meta['topics']}
        assert selected <= parent_leaves
        assert {o.rsplit('.',1)[0] for o in selected} == set(meta['topics'])
        prereqs = list(graph[pid]['requires']) + EXTRA.get(short, [])
        prereqs = list(dict.fromkeys(prereqs))
        artifacts = [p+'/accepted-component-and-dossier' for p in prereqs]
        if wave=='P4': artifacts += ['PR-P3-02 full CRC-frame/streaming component and contract digest (the narrower integration.p3 artifact is insufficient)']
        if wave=='P5': artifacts += ['PR-P1-02 bounded storage/queue interfaces and owner ledger', 'PR-P3-02 full CRC-frame/streaming component with delivery/backpressure contract']
        artifacts += [pid+' learner source, headers and source-bound component dossier',
                      'For every reused component: producer project ID, source hash, public contract revision, dependency profile and actual acceptance evidence; missing evidence remains a prerequisite gap.']
        sources = [dict(id=i,path=resources[i][0],passages=resources[i][1]['locators']) for i in SOURCES[wave]]
        bindings = [pin(project,'Complete metadata, contract, inputs, steps, failure cases, acceptance and transfer'),
                    pin(ROOT/'curriculum/guides/integrations.md',wave+' shared cross-component contract and protocol requirements'),
                    pin(ROOT/'curriculum/guides/audit.md','Dependency/ownership/evidence audit rules'),
                    pin(ROOT/meta['fixtures'][0],'Complete synthetic input and expected model output'),
                    pin(ROOT/'tools/verify_integration_fixtures.py','Independent queue/binary/wire/irq/pressure/fir computation branches'),
                    pin(ROOT/'labs/integration/CONTRACTS.md',wave+' handoff contract and common live-object/nonoverlap/ownership requirements'),
                    pin(ROOT/'labs/integration/include/integration.h','Complete public orchestrator types and function declarations'),
                    pin(ROOT/'labs/integration/tests/contracts.c',wave+' positive and negative test scopes; not the full checkpoint implementation'),
                    pin(ROOT/'assessor/integration/integration.c',wave+' actual orchestrator source and component ownership/progress handoffs'),
                    pin(ROOT/'curriculum/v4/units.json','All assigned checkpoint unit objective/import/export/test mappings')]
        teaching = []
        for domain in sorted({t[:3] for t in meta['topics']}):
            topics = [t for t in meta['topics'] if t.startswith(domain+'.')]
            path = ROOT/'curriculum/modules'/(domain+'.md')
            bindings.append(pin(path,'Substantive teaching sections '+', '.join(topics)))
            teaching.append(dict(path=path.relative_to(ROOT).as_posix(),anchors=topics))
        fixture = 'fixtures/legacy_checkpoints/'+wave+'.json'
        supplied = [project.relative_to(ROOT).as_posix(), meta['fixtures'][0], fixture,
                    'curriculum/guides/integrations.md','labs/integration/README.md',
                    'labs/integration/include/integration.h','tools/verify_legacy_checkpoint_fixtures.py']
        # All six tiny fixtures are supplied so the exported standalone oracle can run.
        supplied += ['fixtures/legacy_checkpoints/P'+str(i)+'.json' for i in range(1,7)]
        supplied = list(dict.fromkeys(supplied))
        criteria, analysis = [], ['# '+pid+' — composition assessor analysis','','Assessor only. Authored analysis is not a learner attempt, independent human review, target execution or physical measurement. The parent implementation remains required.','',LIMIT[wave],'']
        for index,row in enumerate(cards):
            oid=row['objective'];cid='AX-'+pid+'-'+oid;aid=oid.lower().replace('.','')
            variant=row['variant'];variant_prereqs=[]
            if pid=='PR-P1-02' and oid=='T21.04.b':
                variant += ' In a later hosted revisit after PR-T66-01, adapt the completed OwOwner construction/destruction protocol, compare its nonfailing/nonreentrant release assumptions with asynchronous unregister, and measure added control bytes/callback work. ISO C does not enforce move-only ownership or automatic destruction; do not link this hosted comparison into the strict core without a new dependency audit.'
                variant_prereqs=['PR-T66-01/accepted-component-and-dossier']
                supplied += ['labs/ownership/CONTRACTS.md','curriculum/projects/PR-T66-01.md']
                bindings.append(pin(ROOT/'labs/ownership/CONTRACTS.md','Objects and permitted calls; ownership invariants; prefix construction and cleanup assumptions'))
            if pid=='PR-P2-02' and oid=='T21.02.b':
                variant += ' After PR-T65-03, adapt the completed progress/status product to this writer, preserving accepted count and first cause. Measure result size, branch/callback count and additional copying under the same contract. A C tag/accessor protocol does not automatically clean up, enforce exhaustive matching or guarantee a language-specific Result layout.'
                variant_prereqs=['PR-T65-03/accepted-component-and-dossier']
                supplied += ['labs/errors/CONTRACTS.md','curriculum/projects/PR-T65-03.md']
                bindings.append(pin(ROOT/'labs/errors/CONTRACTS.md','Live-object/result-tag/cleanup contracts and representation/cost obligations'))
            facet=dict(teaching='Actual '+oid.rsplit('.',1)[0]+' module section, parent contract and listed inspected source passages.',
                       implementation_or_analysis='The parent implementation stays required. This criterion explicitly assesses the component interaction in task and D2; it adds no historical unit/hour.',
                       boundary_adversarial='The concrete refused, partial, stale, misaligned, exhausted, delayed or numerically changed case in task has its own expected causal argument.',
                       independent_assessment='Level2 requires this objective evidence; level3 requires the named changed-condition attempt after at least seven days with actual assistance/reviewer provenance. No independence result is inferred from authorship.',
                       remaining='Full learner construction/diagnosis and its evidence review, unavailable target/source obligations and delayed transfer remain unattempted.')
            task=row['task']+' Supplied model inputs: '+meta['fixtures'][0]+' and '+fixture+'. Use the exact prerequisite dossiers listed below; additional inputs in this task are normative for this criterion.'
            c=dict(id=cid,title=leaves[oid],objective=oid,families=list(FAMILIES[short][index]),source_records=sources,
                   task=task,purpose=row['misconception'],
                   prerequisites=dict(knowledge=['The parent topics '+', '.join(meta['topics'])+' and their retained prerequisite chain','C17 bounds/lifetime/ownership, chosen dependency profile, and the distinction among ISO C, ABI, OS, library, ISA and board rules'],artifacts=artifacts),
                   reasoning=[row['misconception']+' Identify the smallest state or trace that refutes this explanation.',
                              'State input/output representations, valid extents, preconditions, postconditions, owner/borrow transitions and commit points. Preserve each producer contract or specify an explicit adapter.',
                              'Derive failure/partial-progress/cleanup behavior and the relevant time, storage and work bounds. Explain genuinely inapplicable quantities instead of asserting invented guarantees.'],
                   deliverables=dict(D1='Sealed prediction, ownership/state diagram and causal argument for: '+row['misconception'],
                                     D2=task,
                                     D3='Source/adapter revisions, exact environment/commands, independently expected and actual traces, boundary/failure cases, and a criterion-specific falsification control. Count retained resources/copies/callback work where applicable; retain missing observations honestly.',
                                     D4=variant),
                   verification=dict(procedure='Predict before running. Exercise every task clause with normal/boundary/failure and falsification cases. Compare output plus internal ownership/state invariants through an instrumented public contract; do not rely on a checksum alone. Run the supplied host oracle separately and label its evidence as a bounded model.',
                                     expected=row['expected'],
                                     **{'pass':'Every task clause and expected distinction is supported by source-bound learner evidence. Wrong ownership/failure semantics, erased accepted progress, unsupported target claims or the stated misconception fail this criterion regardless of test count.'},
                                     limits=LIMIT[wave]+' New host model checks do not execute the full new criterion; finite tests do not prove absolute mastery.'),
                   rubric={'0':'Missing, unsafe or incorrect artifacts; violated containment or false semantic guarantee.',
                           '1':'Shows an ordinary example but cannot diagnose the stated misconception or changed interaction.',
                           '2':row['expected']+' Supplies every task artifact and a discriminating negative control under the declared contract.',
                           '3':variant+' Predicts, implements/derives and verifies the changed obligations in an honestly recorded delayed independent attempt.'},
                   hints=HINTS[short],variants=[dict(id=cid+'-DELAYED',minimum_days=7,task=variant,prerequisite_artifacts=variant_prereqs)],
                   assessor_anchor='#'+aid,facet_scope=facet,security=security(oid))
            criteria.append(c)
            analysis += ['<a id="'+aid+'"></a>','## '+oid+' — '+leaves[oid],'',row['expected'],'','Reject: '+row['misconception'],'',
                         'Discriminator: require the first divergent state/ownership edge/output or assumption, not only a final checksum. Check the actual producer and adapter revisions, the independence of the expected result, and whether the claimed environment was executed. '+LIMIT[wave],
                         '', 'Delayed check: '+variant,'']
            trace.append(dict(source_records=sources,topic=oid.rsplit('.',1)[0],objective=oid,criterion=cid,project=pid,learner_artifacts=c['deliverables'],assessment=dict(rubric=c['rubric'],variant_ids=[v['id'] for v in c['variants']]),facet_scope=facet))
            pending.append(dict(project=pid,criterion=cid,status='unattempted',required=True,work='Complete the actual learner implementation/analysis, criterion evidence review and named delayed transfer; do not equate host model validation with completion.'))
        gaps=['Current exact published C17/C23 passages beyond the inspected C11 draft remain separate research obligations.',
              'Learner prerequisite artifact hashes and acceptance receipts are required at attempt time; no supplied learner portfolio was inspected.', LIMIT[wave],
              'No new complete checkpoint implementation campaign, independent human assessment or delayed learner attempt is claimed.']
        if wave=='P2':gaps += ['Exact selected x86-64 Linux process-entry ABI and complete syscall signal/restart semantics must be inspected before the corresponding real target claim.']
        if wave=='P4':gaps += ['Precise F439 UART TXE/TC/DMA completion interaction and current board/errata passages need additional inspection; selected RM0090 receiver/DMA pages are narrower.', 'Dedicated-board reset, IRQ/DMA behavior, quiescence and physical timing remain unexecuted required gates.']
        if wave=='P6':gaps += ['Exact selected SIMD ISA/OS-state dispatch requirements and actual physical performance remain uninspected/unexecuted gates where absent.']
        record=dict(schema_version=1,project=pid,domain='INTEGRATION',title=meta['title']+' — explicit composition assessments',required=True,
                    shared_contracts=['curriculum/v5/expansion/legacy_assessments/shared-contract.md','labs/integration/CONTRACTS.md'],
                    baseline_binding=dict(files=bindings,teaching=teaching,units=[u['id'] for u in pu],inspection_limits=['Complete parent brief, actual assigned teaching sections, shared integration guide, six synthetic fixtures and complete orchestrator/header/contracts/tests inspected.', 'Assigned unit objective/import/export/test mappings inspected; no historical learner portfolio or complete target campaign was audited.', 'Only exact primary passages in source_records are assigned; no bibliography-only inspection claim.']),
                    environment=dict(language='Active C17. N1570 is explicitly a C11 draft comparison; compiler extensions, ABI/OS/ISA/library and board contracts remain separate authorities.',target=TARGET[wave],profile=meta['profile'],
                                     dependencies='Keep the retained profile and producer restrictions. Implement the taught mechanism from permitted primitives; no container/parser/allocator/filter library substitutes. Strict target images audit implicit helpers and all resolved providers. Python/zlib/oracles and diagnostic tools are host verification only. Modern hosted adapter revisits remain separate dependency scopes.',
                                     toolchain=['Record compiler/version/flags/optimization/data model and actual linker map/symbol closure.', 'Record OS/ABI/ISA/emulator/board and clock evidence for the specific claim; unavailable tools or targets remain missing observations.']),
                    source_records=sources,reference_gaps=gaps,supplied_artifacts=list(dict.fromkeys(supplied)),
                    prerequisite_artifacts=artifacts,additional_required_projects=EXTRA.get(short,[]),
                    execution_support=dict(kind='Retained bounded integration anchor plus new independent synthetic host model',scope=LIMIT[wave],tests=['integration.'+wave.lower()],fresh_execution='New fixture oracle recomputation only, recorded in scoped validation. Full checkpoint criteria remain unexecuted.'),
                    criteria=criteria,assessor_analysis='assessor/legacy_depth/'+pid+'.md',
                    status=dict(content='authored',execution='Bounded fixture models actually checked separately; no new full learner/checkpoint target execution.',independent_review='not-run',learner_assessment='not-assessed',limitations=['Accepted identities, unit bytes, descriptive historical hours and receipts are preserved.','Authored delayed opportunities are not completed assessments.','Finite passing tests do not prove absolute mastery.']))
        audit.append(dict(project=pid,topics=meta['topics'],selected_objectives=sorted(selected),baseline_bindings=bindings,
                          prerequisite_projects=prereqs,additive_guide_dependencies=EXTRA.get(short,[]),
                          teaching='Actual substantive parent-topic sections retained',construction='Retained checkpoint contract plus individually specified component interaction tasks',
                          boundary='Distinct partial-progress, ownership, recovery, capacity, environment or numeric counterexamples',
                          independent_assessment_before='Retained general acceptance/transfer wording lacked these explicit composition criteria and objective-specific discriminators',
                          expansion='Individually authored criteria with concrete task/expected arguments, graduated hints, separate assessor analysis and seven-day variants',existing_anchor_limit=LIMIT[wave],required_remaining=gaps))
        for oid in sorted(parent_leaves):
            dispositions.append(dict(project=pid,objective=oid,disposition='composition-criterion-authored' if oid in selected else 'retained-parent-domain-obligation',criterion='AX-'+pid+'-'+oid if oid in selected else None,
                                     scope='Selected leaves assess concrete integration interactions; nonselected leaves remain in their domain projects/supplements and are not newly claimed as checkpoint-assessed.',required=True))
        write(ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json'),attach_linux_followthrough(attach_numeric_followthrough(attach_protocol_followthrough(attach_firmware_followthrough(attach_writer_followthrough(attach_entry_followthrough(attach_memory_followthrough(attach_successor_support(record)))))))))
        (ROOT/record['assessor_analysis']).write_text('\n'.join(analysis)+'\n',encoding='utf-8',newline='\n')
    write(HERE/'depth_audit.json',dict(schema_version=1,status='author passage/depth audit',projects=audit))
    write(HERE/'traceability.json',dict(schema_version=1,scope='Actual passages → parent topic/objective → criterion → learner artifacts → rubric/delayed assessment. Authored opportunity, not learner evidence.',rows=trace))
    write(HERE/'objective_dispositions.json',dict(schema_version=1,rows=dispositions))
    write(HERE/'remaining_work.json',dict(schema_version=1,required=True,criteria=pending,external_and_experiment_gaps=[dict(project=a['project'],required=True,work=a['required_remaining']) for a in audit],completion_policy='Authored contract completion and actual learner/target/independent evidence are separate. No missing target observation is closed by a host model.'))
    gate_paths=[HERE/'resources.json',HERE/'AUDIT.md',HERE/'cards.py']
    gate_paths += [ROOT/p for p in REGISTERS[1:] if p.startswith('research/v5/expansion/')]
    write(HERE/'research_gate.json',dict(status='passage-inspected',domains=sorted({r['objective'][:3] for cards in data.values() for r in cards}),projects=list(data),
          inspection_kind='Author inspection of exact primary and baseline passages; not independent human acceptance.',
          research_input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in gate_paths}))
    print(json.dumps(dict(projects=len(data),criteria=len(trace),selected_objectives=len({r['objective'] for r in trace}),parent_leaf_instances=len(dispositions),topics=len({r['topic'] for r in trace}))))


if __name__=='__main__':
    main()
