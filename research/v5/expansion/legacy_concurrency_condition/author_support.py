"""Bind exact original condition criteria and delayed variants to current author evidence."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_concurrency_condition/support.json'
CONTRACT='labs/execution/sync_depth/CONDITION.md'
TARGET_CONTRACT=CONTRACT
HOST='reviews/v5/expansion/legacy_concurrency_sync/condition_campaign.json'
TARGET='reviews/v5/expansion/legacy_concurrency_sync/condition_target.json'
PROJECT='PR-T51-05'
SCOPES={
 'T51.05.a':'Actual Mesa stolen-item/recheck and unremembered/early-under-lock signals;three consumers under broadcast claim exactly one value and rewait correctly.',
 'T51.05.b':'Actual timeout/cancel/close selects a pending status before ownership restoration;deadline5 remains pending at99;matching-mutex check and destruction refusal preserve lifetime.',
 'T51.05.c':'Actual capacity1/2 copied-value/request ledger, spurious wake and shutdown/drain;two-predicate single-signal stall versus broadcast progress with identical accepted3/consumed3.'}



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
    links.append({'path':SUPPORT,'criterion_ids':ids,'scope':'Actual CMX predicate/request composition, terminal reacquisition and shutdown/drain; no independent or physical claim.'})
    for contract in (CONTRACT,TARGET_CONTRACT):
        if contract not in record['shared_contracts']:record['shared_contracts'].append(contract)
    public=['labs/execution/sync_depth/'+name for name in
            ('condition.h','condition.c','condition_tests.c','condition_m4.c','CONDITION.md','run_condition.py')]
    public += ['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in public:
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record


def register():
    definitions=[('COURSE','Actual CMX Mesa condition and PR-T51-05 source contract','curriculum/projects/PR-T51-05.md','course',
         ['assessor/execution/cmx.c wait_on/relock/abandon/tick/cancel and condition wait/signal/close; mutex close retains task.aux references',
          'labs/execution/include/cmx.h serialized wait/task/condition controls; labs/execution/tests/contracts.c condition timeout and faults lifetime cases',
          'curriculum/modules/T51.md T51.05 and concurrent-system-contracts primitive table; complete PR-T51-05 brief/fixture and three original criteria/variants/assessor arguments'],
         'Bounded actual original course-source reads before successor construction. Existing external passage references remain in the retained criterion register; no new external-source inspection is claimed.')]
    rows=[]
    for suffix,title,url,kind,locators,limits in definitions:
        rows.append({'id':'R-LCCOND-'+suffix,'title':title,'authors':['C Mastery' if kind=='course' else 'Arm CMSIS authors'],
          'url':url,'kind':kind,'edition':'As identified in title and exact locator','domains':['T51'],'locators':locators,
          'access':'Actual bounded source/documentation passage inspected','role':'oracle','assignable':True,
          'reason':'Support exact enrollment, predicate recheck, terminal ownership restoration and copied-value ledger arguments.',
          'prerequisites':['PR-T51-01 wait protocol, PR-T51-03 owned mutex and PR-T51-04 urgency/ownership distinction'],
          'inspection':{'date':'2026-10-17','method':'Read actual bounded source/manual passages before adapter/target construction.',
                        'scope':'; '.join(locators),'limits':limits}})
    write(BASE/'resources.json',{'schema_version':1,'resources':rows})
    write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T51'],
        'inspection_kind':'Author bounded primary/course-source inspection; no independent reviewer claim',
        'research_input_sha256':{'research/v5/expansion/legacy_concurrency_condition/resources.json':hashlib.sha256((BASE/'resources.json').read_bytes()).hexdigest()}})


def main():
    register();sys.path.insert(0,str(ROOT/'tools'))
    from check_condition_depth import validate_host,validate_target
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
                'implementation':{'state':'complete-for-declared-reference-and-changed-condition','evidence':['assessor/legacy_concurrency_sync/condition.c','assessor/execution/cmx.c']},
                'author_validation':{'state':'passed-hosted-and-emulated-with-stated-scope','evidence':[HOST,TARGET]},
                'external_review':{'state':'not-run-for-this-successor','evidence':[]},
                'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-run-no-physical-claim','evidence':[]}},
            'variant_ids':[v['id'] for v in criterion['variants']],
            'required_environment':'Explicit completed CMX import; C17 serialized core; owned QEMU Cortex-M4 execution with declared helper closure.',
            'external_blockers':['Independent review and actual delayed unaided learner attempts.','Physical target observation only for a physical claim.'],
            'evidence_limits':['No task context-switch, multicore scheduling, physical timing or certified execution-time bound is inferred.',
                               'Controlled Mesa/terminal/broadcast schedules are declared subsets; no universal concurrency, liveness or mastery proof.']}
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
          'research/v5/expansion/legacy_concurrency_condition/author_support.py','research/v5/expansion/legacy_concurrency_condition/resources.json',
          'tools/check_condition_depth.py']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
        'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
        'campaigns':[{'producer':'tools/run_condition_depth.py','report':HOST},{'producer':'tools/run_condition_target.py','report':TARGET}],
        'scope':'Only original T51.05 criteria and exact delayed variants; no whole-T51 or other project closure.',
        'remaining_author_work':[],'remaining_external_work':['Independent review, unaided delayed learner attempts and physical observations where claimed.']})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))


if __name__=='__main__':main()
