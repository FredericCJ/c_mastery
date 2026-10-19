"""Adjudicate explicit external-evidence obligations without closing their gates.

The declarations below are reviewed scope decisions, not completion inferred
from an ID prefix, an empty backlog, a heading or a passing structural check.
Executable constituent obligations retain their separate producer packets.
"""
from common import ROOT, read_json, dump, sha

BASE='research/v5/expansion/external_obligations'
STANDARDS='''G-ABSTRACTION-C17 G-B01-ISO17 G-CONTRACT-C17 G-EVENTS-PUBLISHED-C17
G-FAILURE-C17 G-LLIB-C17-ENCRYPTED G-LLIB-C90-TEXT G-SOURCE-PUBLISHED-C23
G-T76-C17-SOURCE G-T76-C23-SOURCE LSYS-ISO LSYS-T11-ISO NA-ISO NC-ISO NI-ISO NW-ISO
T67-C17-REFERENCE T67-C17-TEXT T67-C23-REFERENCE T67-C23-TEXT
T68-C17-REFERENCE T68-C17-TEXT T68-C23-REFERENCE T68-C23-TEXT T74-REF
T76-C17-TEXT T76-C23-TEXT XD-ISO XF-ISO XI-ISO XK-ISO published-TS6010'''.split()
PERFORMANCE='''G-LCON-LEARNER G-LDEPTH-API-ASSESS G-LDEPTH-DIAGNOSTICS-ASSESS
G-LEARNER LRUN-01 LSYS-LEARNER MBR-INDEPENDENT NW-TCP-LEARNER SD-INDEPENDENT'''.split()
PHYSICAL={
 'G-CAN-TWO-NODE-DEVICE':['labs/hosted_systems/can_depth/BENCH.md','labs/hosted_systems/can_depth/bench_plan.json','labs/hosted_systems/can_depth/check_bench.py'],
 'G-LCON-PLATFORM':['curriculum/guides/physical-evidence.md','curriculum/guides/interrupt-handoff-board-protocol.md'],
 'G-LDEPTH-PHYSICAL':['curriculum/guides/physical-evidence.md','labs/testing_followthrough/BOARD_CAPTURE.md'],
 'G-PHYSICAL':['curriculum/guides/physical-evidence.md','labs/amp_followthrough/BOARD_CAPTURE.md'],
 'G-PL-G474-FULL-PORT-PHYSICAL':['labs/g474_uart_followthrough/BOARD_CAPTURE.md'],
 'NA-PHYSICAL':['curriculum/guides/network-arp-board-protocol.md'],
 'NI-PHYSICAL':['curriculum/guides/network-ip-board-protocol.md'],
 'NW-PHYSICAL':['curriculum/guides/network-ip-board-protocol.md','curriculum/guides/network-arp-board-protocol.md'],
 'NW-TCP-PHYSICAL':['curriculum/guides/network-tcp-board-protocol.md','fixtures/networking/tcp_physical_capture.json','tools/check_network_tcp_capture.py'],
 'T74-HARDWARE':['labs/object_runtime/CONTRACTS.md','curriculum/guides/physical-evidence.md'],
 'XD-PHYSICAL':['curriculum/guides/interrupt-lease-board-protocol.md'],
 'XD-RESET-DOMAIN':['curriculum/guides/interrupt-lease-board-protocol.md'],
 'XF-PHYSICAL':['labs/interrupt_depth/frame/CONTRACTS.md','curriculum/guides/physical-evidence.md'],
 'XI-PHYSICAL':['curriculum/guides/interrupt-handoff-board-protocol.md'],
 'XK-CLOCK-QUALIFICATION':['curriculum/guides/interrupt-time-board-protocol.md'],
 'XK-PHYSICAL':['curriculum/guides/interrupt-time-board-protocol.md']}

def original(row):
    origin=next(o for o in row['origins'] if o.get('role')!='reference')
    value=read_json(ROOT/origin['path'])
    for bit in origin['pointer'].split('/')[1:]:
        key=bit.replace('~1','/').replace('~0','~')
        value=value[int(key)] if isinstance(value,list) else value[key]
    return origin,value

def make(ident,category,artifacts,missing,attempts,finish):
    row=INDEX[ident];origin,record=original(row)
    files=list(dict.fromkeys([origin['path'],*artifacts]))
    for name in files:assert (ROOT/name).is_file(),name
    # Reference/observation obligations are not executable implementation gates.
    # Their individual implementation children remain separately adjudicated.
    dimensions={name:dict(state='not-applicable-to-this-evidence-obligation',evidence=[])
                for name in ['teaching','implementation','author_validation','external_review','independent_learner','physical_observation']}
    if category=='standard':
        dimensions['teaching']=dict(state='bounded-draft-or-source-support;exact-final-text-open',evidence=files)
        dimensions['author_validation']=dict(state='exact-required-edition-not-inspected',evidence=files)
    elif category=='physical':
        dimensions['teaching']=dict(state='qualification-and-capture-protocol-authored',evidence=artifacts)
        dimensions['implementation']=dict(state='supporting-code-assessed-by-constituent-obligations;no-board-port-inferred',evidence=artifacts)
        dimensions['author_validation']=dict(state='protocol-reviewed;physical-result-not-inferred',evidence=files)
        dimensions['physical_observation']=dict(state='not-run;exact-equipment-and-capture-required',evidence=[])
    elif category=='review':dimensions['external_review']=dict(state='not-run;non-author-required',evidence=[])
    else:
        dimensions['external_review']=dict(state='not-run',evidence=[])
        dimensions['independent_learner']=dict(state='not-assessed;actual-delayed-attempt-required',evidence=[])
    return dict(id=ident,classification=category,originating_record=dict(**origin,record=record),
        dimensions=dimensions,closure_rationale='Gate remains open in its exact evidence dimension. Original record and constituent author work are preserved.',
        prerequisites=row['prerequisites'],affected_consumers=row['affected_consumers'] or [ident],
        required_environment=missing,validation_method=finish,
        external_blockers=[dict(missing_prerequisite=missing,attempts=attempts,
            completed_author_support=files,procedure_to_finish=finish)],
        evidence_limits=['No source inspection, measurement, reviewer independence or learner result inferred from a bibliography, authored protocol or synthetic capture.'],
        artifact_sha256={p:sha(ROOT/p) for p in files})

def generate():
    global INDEX
    INDEX={r['id']:r for r in read_json(ROOT/'curriculum/v5/expansion/work_inventory.json')['obligations']}
    updates=[]
    for ident in STANDARDS:
        edition=('ISO C90 header-list passage' if ident=='G-LLIB-C90-TEXT' else
                 'published ISO/IEC TS6010 provenance clauses' if ident=='published-TS6010' else
                 'exact published ISO C edition and named clauses specified by the original record')
        updates.append(make(ident,'standard',['research/v5/expansion/followthrough/sources.md','research/v5/expansion/followthrough/source_access_successor.json'],
            'Authorized readable '+edition,
            'Retained source-access attempts: protected/unreadable draft retrieval is not final-standard access; N1570/N3096/N3231 keep their identified draft status. No purchase or access-control circumvention performed.',
            'Obtain authorized exact edition; record edition/hash and actually read the affected clauses; add a versioned comparison and rerun only consumers whose semantics change. A draft/compiler probe cannot close this gate.'))
    for ident in PERFORMANCE:
        updates.append(make(ident,'independent-performance',['curriculum/guides/assessment.md'],
            'A real independent learner submission and assessor, including the original at-least-seven-day changed-condition attempt',
            'Author packages and reference executions exist in exact child dispositions; no learner was impersonated and no elapsed assessment interval was fabricated.',
            'Administer the original criterion-specific ordinary and withheld transfer packets; retain dated learner artifact identities, assistance, per-objective rubric decisions and actual interval.'))
    updates.append(make('G-REVIEW','review',['curriculum/guides/assessment.md'],
        'A non-author technical reviewer with a declared inspected scope',
        'Implementing-agent and parallel author checks are retained as author validation only.',
        'Record reviewer identity/independence, inspected source revision, findings and verified corrections; do not promote parallel authorship to independent review.'))
    for ident,artifacts in PHYSICAL.items():
        updates.append(make(ident,'physical',artifacts,
            'The exact selected dedicated board/part/revision, authoritative wiring/clock/register sources, qualified instruments, authorized operator and raw observations named in the retained record',
            'Author-side native, model, linked-image and emulator evidence is classified in constituent packets; no hardware flashing, host policy change or physical capture was performed.',
            'Follow the linked qualification and containment protocol; retain raw source-bound captures, calibration/uncertainty, positive/failure/changed-condition observations and criterion-specific review. Reject invented values and emulator-to-board extrapolation.'))
    dump(ROOT/BASE/'completion_dispositions.json',dict(schema_version=1,
        scope='Explicit 58 source or external-evidence origins, retained as open evidence dimensions. This is not whole-curriculum acceptance.',updates=updates,
        source_sha256={'tools/reconcile_external_obligations.py':sha(ROOT/'tools/reconcile_external_obligations.py')}))
    print(dict(external_obligations=len(updates),source=len(STANDARDS),physical=len(PHYSICAL),independent=len(PERFORMANCE),non_author_review=1))

if __name__=='__main__':generate()
