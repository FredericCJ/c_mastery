"""Bind exact original PI criteria and delayed variants to new author evidence."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_concurrency_sync/pi_support.json'
CONTRACT='labs/execution/sync_depth/PI.md'
TARGET_CONTRACT='labs/execution/sync_depth/PI_TARGET.md'
HOST='reviews/v5/expansion/legacy_concurrency_sync/pi_campaign.json'
TARGET='reviews/v5/expansion/legacy_concurrency_sync/pi_target.json'
PROJECT='PR-T51-04'
SCOPES={
 'T51.04.a':'Actual CMX L9/M7/H3 donation, H5 direct edge, timeout/cancel restoration and exact original priority-convention translation.',
 'T51.04.b':'Bounded independently derived reachability recomputation with changed base priorities, multiple owned locks, selected ceilings, cycle/refusal atomicity and full16-task chain.',
 'T51.04.c':'Identical actual CMX scheduler workloads with/without donation, raw nested-critical/extension cost observations and four restricted recurrence/tick-simulation comparisons; no certified timing bound.'}


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
    links.append({'path':SUPPORT,'criterion_ids':ids,'scope':'Actual CMX import, priority extension, finite oracle, restricted blocking transfer and emulated target; no independent or physical claim.'})
    for contract in (CONTRACT,TARGET_CONTRACT):
        if contract not in record['shared_contracts']:record['shared_contracts'].append(contract)
    public=['labs/execution/sync_depth/'+name for name in
            ('pi.h','pi.c','pi_tests.c','pi_model.c','pi_m4.c','PI.md','PI_TARGET.md','run_pi.py','blocking.py','blocking.json')]
    public += ['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in public:
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record


def register():
    resources={'schema_version':1,'resources':[{
        'id':'R-LCSYNC-PI','title':'Actual course CMX priority inheritance and original PR-T51-04 assessment',
        'authors':['C Mastery'],'url':'curriculum/projects/PR-T51-04.md','kind':'course','edition':'Retained baseline plus separately named successor',
        'domains':['T51'],'locators':['assessor/execution/cmx.c priority(), mutex lock/unlock/close, cancellation, tick, relock and cmx_schedule',
          'labs/execution/include/cmx.h task/base/effective fields, mutex registry and external serialization contract',
          'fixtures/os_v3/PR-T51-04.json complete larger-is-more-urgent finite example',
          'curriculum/projects/PR-T51-04.md complete brief; curriculum/modules/T51.md T51.04 teaching passage; original three supplemental criteria and delayed tasks'],
        'access':'Actual bounded course sources read before extension authorship','role':'oracle','assignable':True,
        'reason':'The new graph algorithm and restricted recurrence are original course derivations, not relabeled unseen external papers.',
        'prerequisites':['PR-T51-01','PR-T51-03','PR-T50-06'],
        'inspection':{'date':'2026-10-17','method':'Read actual source bodies, header, fixture and objective-specific contract.',
            'scope':'Only the named synchronization and priority/scheduler passages; the complete fixture and exact assigned criteria.',
            'limits':'No new inspection of an external response-time or priority-inheritance paper is claimed; original restricted analysis is labeled as such.'}}]}
    write(BASE/'resources.json',resources)
    write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T51'],
        'inspection_kind':'Author bounded course-source inspection; no independent reviewer claim',
        'research_input_sha256':{'research/v5/expansion/legacy_concurrency_sync/resources.json':hashlib.sha256((BASE/'resources.json').read_bytes()).hexdigest()}})


def main():
    register();sys.path.insert(0,str(ROOT/'tools'))
    from check_pi_depth import validate_host,validate_target
    host=json.loads((ROOT/HOST).read_text(encoding='utf-8'));target=json.loads((ROOT/TARGET).read_text(encoding='utf-8'))
    validate_host(host);validate_target(target)
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
    record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],
            'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
            'dimensions':{
                'teaching':{'state':'complete-for-declared-contract','evidence':[CONTRACT,TARGET_CONTRACT]},
                'implementation':{'state':'complete-for-declared-reference-and-changed-condition','evidence':['assessor/legacy_concurrency_sync/pi.c','assessor/execution/cmx.c','labs/execution/sync_depth/blocking.py']},
                'author_validation':{'state':'passed-hosted-and-emulated-with-stated-scope','evidence':[HOST,TARGET]},
                'external_review':{'state':'not-run-for-this-successor','evidence':[]},
                'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-run-no-physical-claim','evidence':[]}},
            'variant_ids':[v['id'] for v in criterion['variants']],
            'required_environment':'Explicit completed CMX import; C17 host/independent finite oracle; owned QEMU Cortex-M4 for logical-task target observations.',
            'external_blockers':['Independent review and actual delayed unaided learner attempts.','Physical target observation only for a physical claim.'],
            'evidence_limits':['No task context-switch, multicore scheduling, physical timing or certified execution-time bound is inferred.',
                               'Finite graph and workload universes are exact declared subsets, not a universal scheduling or mastery proof.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],
                original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
                variant_readiness='Implemented and author-executed under the declared changed assumptions; delayed learner assessment is separate.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-'+PROJECT,
        original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
        closure_rationale='All three original criteria and their exact changed conditions have bounded current author references; learner/review/physical gates remain separate.',
        criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    pins=[HOST,TARGET,CONTRACT,TARGET_CONTRACT,canonical.relative_to(ROOT).as_posix(),
          'research/v5/expansion/legacy_concurrency_sync/author_support.py','research/v5/expansion/legacy_concurrency_sync/resources.json',
          'tools/check_pi_depth.py']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
        'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
        'campaigns':[{'producer':'tools/run_pi_depth.py','report':HOST},{'producer':'tools/run_pi_target.py','report':TARGET}],
        'scope':'Only original T51.04 criteria and exact delayed variants; no whole-T51 or T52 closure.',
        'remaining_author_work':[],'remaining_external_work':['Independent review, unaided delayed learner attempts and physical observations where claimed.']})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))


if __name__=='__main__':main()
