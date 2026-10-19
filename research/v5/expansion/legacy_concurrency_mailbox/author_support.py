"""Bind exact original mailbox criteria and delayed variants to current author evidence."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_concurrency_mailbox/support.json'
CONTRACT='labs/execution/sync_depth/MAILBOX.md'
TARGET_CONTRACT=CONTRACT
HOST='reviews/v5/expansion/legacy_concurrency_sync/mailbox_campaign.json'
TARGET='reviews/v5/expansion/legacy_concurrency_sync/mailbox_target.json'
PROJECT='PR-T51-07'
SCOPES={
 'T51.07.a':'Actual CMX capacity0/1 transfers and receiver-first borrowed-copy boundary;transport completion remains distinct from copied reply and application effect.',
 'T51.07.b':'Two fixed outstanding clients with checked slot/generation/session identity;selected timeout before polling, g1 timeout/g2 stale-reply isolation, unchanged short-buffer refusal and finite generation/session exhaustion.',
 'T51.07.c':'Actual transfer/cancel matrix and synthetic duplicate-effect comparison;real CLAck four-entry-per-client replay/eviction binding, stable-ID retry and stale high-water refusal.'}



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
    links.append({'path':SUPPORT,'criterion_ids':ids,'scope':'Actual CMX queue/semaphore and CLAck receipt composition, transfer identity and bounded replay; no independent or physical claim.'})
    for contract in (CONTRACT,TARGET_CONTRACT):
        if contract not in record['shared_contracts']:record['shared_contracts'].append(contract)
    public=['labs/execution/sync_depth/'+name for name in
            ('mailbox.h','mailbox.c','mailbox_tests.c','mailbox_m4.c','MAILBOX.md','run_mailbox.py')]
    public += ['labs/concurrency_lifecycle_depth/include/cl.h','labs/concurrency_lifecycle_depth/src/core.c','labs/concurrency_lifecycle_depth/CONTRACTS.md']
    public += ['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in public:
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record


def register():
    definitions=[('COURSE','Actual CMX mailbox/rendezvous, CLAck receipt and PR-T51-07 contracts','curriculum/projects/PR-T51-07.md','course',
         ['assessor/execution/cmx.c queue send/recv/close/abort including capacity0 direct match; semaphore give/take, wait_on/cancel and task result publication',
          'labs/concurrency_lifecycle_depth/CONTRACTS.md Application acknowledgement changed condition; actual include/cl.h CLAck declarations and assessor/concurrency_lifecycle_depth/core.c cl_ack_init/request/apply/acknowledge',
          'curriculum/modules/T51.md T51.06/T51.07; complete PR-T51-07 brief/fixture and three original criteria/variants/assessor arguments'],
         'Bounded actual original course-source reads before successor construction. Existing external passage references remain in the retained criterion register; no new external-source inspection is claimed.')]
    rows=[]
    for suffix,title,url,kind,locators,limits in definitions:
        rows.append({'id':'R-LCMB-'+suffix,'title':title,'authors':['C Mastery' if kind=='course' else 'Arm CMSIS authors'],
          'url':url,'kind':kind,'edition':'As identified in title and exact locator','domains':['T51'],'locators':locators,
          'access':'Actual bounded source/documentation passage inspected','role':'oracle','assignable':True,
          'reason':'Support exact borrow/copy/transfer/reply boundaries, finite identity and bounded volatile receipt arguments.',
          'prerequisites':['PR-T51-01 common wait protocol, PR-T51-06 copied/owned messages and its CLAck receipt successor'],
          'inspection':{'date':'2026-10-17','method':'Read actual bounded source/manual passages before adapter/target construction.',
                        'scope':'; '.join(locators),'limits':limits}})
    write(BASE/'resources.json',{'schema_version':1,'resources':rows})
    write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T51'],
        'inspection_kind':'Author bounded primary/course-source inspection; no independent reviewer claim',
        'research_input_sha256':{'research/v5/expansion/legacy_concurrency_mailbox/resources.json':hashlib.sha256((BASE/'resources.json').read_bytes()).hexdigest()}})


def main():
    register();sys.path.insert(0,str(ROOT/'tools'))
    from check_mailbox_depth import validate_host,validate_target
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
                'implementation':{'state':'complete-for-declared-reference-and-changed-mailbox','evidence':['assessor/legacy_concurrency_sync/mailbox.c','assessor/execution/cmx.c','assessor/concurrency_lifecycle_depth/core.c']},
                'author_validation':{'state':'passed-hosted-and-emulated-with-stated-scope','evidence':[HOST,TARGET]},
                'external_review':{'state':'not-run-for-this-successor','evidence':[]},
                'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-run-no-physical-claim','evidence':[]}},
            'variant_ids':[v['id'] for v in criterion['variants']],
            'required_environment':'Explicit completed CMX and CLAck imports; C17 serialized core; owned QEMU Cortex-M4 execution with declared helper closure.',
            'external_blockers':['Independent review and actual delayed unaided learner attempts.','Physical target observation only for a physical claim.'],
            'evidence_limits':['No task context-switch, multicore scheduling, physical timing or certified execution-time bound is inferred.',
                               'Controlled transfer/timeout/restart traces are declared subsets; no authentication, durability, distributed exactly-once or mastery proof.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],
                original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
                variant_readiness='Implemented and author-executed under the declared changed assumptions; delayed learner assessment is separate.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-'+PROJECT,
        original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
        closure_rationale='All three original criteria and their exact changed mailbox variants have bounded current author references; learner/review/physical gates remain separate.',
        criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    pins=[HOST,TARGET,CONTRACT,TARGET_CONTRACT,canonical.relative_to(ROOT).as_posix(),
          'research/v5/expansion/legacy_concurrency_mailbox/author_support.py','research/v5/expansion/legacy_concurrency_mailbox/resources.json',
          'tools/check_mailbox_depth.py']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
        'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
        'campaigns':[{'producer':'tools/run_mailbox_depth.py','report':HOST},{'producer':'tools/run_mailbox_target.py','report':TARGET}],
        'scope':'Only original T51.07 criteria and exact delayed variants; no whole-T51 or other project closure.',
        'remaining_author_work':[],'remaining_external_work':['Independent review, unaided delayed learner attempts and physical observations where claimed.']})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))


if __name__=='__main__':main()
