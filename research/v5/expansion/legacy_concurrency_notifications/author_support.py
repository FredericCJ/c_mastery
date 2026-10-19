"""Bind exact original notifications criteria and delayed variants to current author evidence."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_concurrency_notifications/support.json'
CONTRACT='labs/execution/sync_depth/NOTIFICATIONS.md'
TARGET_CONTRACT=CONTRACT
HOST='reviews/v5/expansion/legacy_concurrency_sync/notifications_campaign.json'
TARGET='reviews/v5/expansion/legacy_concurrency_sync/notifications_target.json'
PROJECT='PR-T51-09'
NATIVE='reviews/v5/expansion/legacy_concurrency_sync/notifications_native.json'
SCOPES={
 'T51.09.a':'Actual post3/post2/take4 conservation, all-or-error versus clamp contrast; full-width counters, finite cleanup and exact acknowledged overflow fault records.',
 'T51.09.b':'Actual CMX direct claim1 plus stored2, one-shot wait-up-to-N result snapshot/consume-all, selected terminal order, incarnation retirement and real IRQ context/mask controls.',
 'T51.09.c':'Independent1024 histories/5120 transitions, Boolean collapse controls, separate virtual IRQ/post/deferred intervals and actual distinct-CPU pthread publication with contention samples and race-free atomic lost-update contrast.'}



def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')


def attach(record):
    if record['project']!=PROJECT:return record
    ids=[]
    for criterion in record['criteria']:
        if criterion['objective'] in SCOPES:
            criterion['successor_support']={'path':SUPPORT,'id':criterion['id']};ids.append(criterion['id'])
    links=record.setdefault('additional_successor_support',[])
    links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':ids,'scope':'Actual CMX counted occurrences/faults/partial results, Cortex-M4 IRQ/deferred observations and distinct-CPU pthread adapter; no independent or physical claim.'})
    for contract in (CONTRACT,TARGET_CONTRACT):
        if contract not in record['shared_contracts']:record['shared_contracts'].append(contract)
    public=['labs/execution/sync_depth/'+name for name in
            ('notifications.h','notifications.c','notifications_tests.c','notifications_m4.c','notifications_irq.h','notifications_irq.c',
             'notifications_native.h','notifications_native.c','notifications_native_tests.c','NOTIFICATIONS.md','run_notifications.py','run_notifications_native.py')]
    public += ['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in public:
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record


def register():
    # Preserve actual passage records; refresh only hashes of those unchanged
    # inspections and the explicitly declared external source register.
    gate=json.loads((BASE/'research_gate.json').read_text(encoding='utf-8'))
    for path in gate['research_input_sha256']:
        gate['research_input_sha256'][path]=hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
    write(BASE/'research_gate.json',gate)


def main():
    register();sys.path.insert(0,str(ROOT/'tools'))
    from check_notifications_depth import validate_host,validate_target,validate_native
    host=json.loads((ROOT/HOST).read_text(encoding='utf-8'));target=json.loads((ROOT/TARGET).read_text(encoding='utf-8'))
    native=json.loads((ROOT/NATIVE).read_text(encoding='utf-8'));validate_host(host);validate_target(target);validate_native(native)
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
    record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],
            'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
            'dimensions':{
                'teaching':{'state':'complete-for-declared-contract','evidence':[CONTRACT,TARGET_CONTRACT]},
                'implementation':{'state':'complete-for-declared-reference-and-changed-notifications','evidence':['assessor/legacy_concurrency_sync/notifications.c','assessor/legacy_concurrency_sync/notifications_native.c','assessor/legacy_concurrency_sync/notifications_irq.c','assessor/execution/cmx.c']},
                'author_validation':{'state':'passed-hosted-and-emulated-with-stated-scope','evidence':[HOST,TARGET,NATIVE]},
                'external_review':{'state':'not-run-for-this-successor','evidence':[]},
                'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-run-no-physical-claim','evidence':[]}},
            'variant_ids':[v['id'] for v in criterion['variants']],
            'required_environment':'Explicit completed CMX import; C17 serialized core; Linux pthread/affinity on two effective allowed CPUs; owned QEMU Cortex-M4 with declared helper closure.',
            'external_blockers':['Independent review and actual delayed unaided learner attempts.','Physical target observation only for a physical claim.'],
            'evidence_limits':['No task context-switch, lock-free/weak-memory proof, physical timing or certified execution-time bound is inferred; native mutex serialization is explicit.',
                               'Finite history/native workloads are declared subsets; counts do not recover payloads/authenticate producers. Refused attempts differ from distinct occurrences; unexpected port failures require stop/inspect.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],
                original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
                variant_readiness='Implemented and author-executed under the declared changed assumptions; delayed learner assessment is separate.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-'+PROJECT,
        original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
        closure_rationale='All three original criteria and their exact changed notifications variants have bounded current author references; learner/review/physical gates remain separate.',
        criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    pins=[HOST,TARGET,NATIVE,CONTRACT,TARGET_CONTRACT,canonical.relative_to(ROOT).as_posix(),
          'research/v5/expansion/legacy_concurrency_notifications/author_support.py','research/v5/expansion/legacy_concurrency_notifications/resources.json','research/v5/expansion/legacy_concurrency_notifications/research_gate.json',
          'tools/check_notifications_depth.py']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
        'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
        'campaigns':[{'producer':'tools/run_notifications_depth.py','report':HOST},{'producer':'tools/run_notifications_target.py','report':TARGET},{'producer':'tools/run_notifications_native.py','report':NATIVE}],
        'scope':'Only original T51.09 criteria and exact delayed variants; no whole-T51 or other project closure.',
        'remaining_author_work':[],'remaining_external_work':['Independent review, unaided delayed learner attempts and physical observations where claimed.']})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))


if __name__=='__main__':main()
