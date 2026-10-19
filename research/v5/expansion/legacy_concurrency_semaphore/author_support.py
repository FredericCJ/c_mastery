"""Bind exact original semaphore criteria and delayed variants to current author evidence."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_concurrency_semaphore/support.json'
CONTRACT='labs/execution/sync_depth/SEMAPHORE.md'
TARGET_CONTRACT='labs/concurrency_lifecycle_depth/wait/CONTRACTS.md'
HOST='reviews/v5/expansion/legacy_concurrency_sync/semaphore_campaign.json'
TARGET='reviews/v5/expansion/legacy_concurrency_sync/semaphore_target.json'
PROJECT='PR-T51-02'
SCOPES={
 'T51.02.a':'Actual CMX/wait permit conservation at initial1/maximum2, direct handoff, overflow, timeout-before-post and UINT_MAX/binary boundaries.',
 'T51.02.b':'Nonblocking try/post plus actual permitted IRQ0 and forbidden urgent IRQ0 to retained-credit/PendSV transfer; exact saved masks and visible admission/cleanup outcomes.',
 'T51.02.c':'All24 two-post/timeout/close histories against a separate ledger; fixed-bound participant scans, priority admission and a100-overtake starvation prefix with unchanged token conservation.'}



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
    links.append({'path':SUPPORT,'criterion_ids':ids,'scope':'Actual CMX/generational-wait imports, permit/deferred-credit ledgers and IRQ/PendSV transfer; no independent or physical claim.'})
    for contract in (CONTRACT,TARGET_CONTRACT):
        if contract not in record['shared_contracts']:record['shared_contracts'].append(contract)
    public=['labs/execution/sync_depth/'+name for name in
            ('semaphore.h','semaphore.c','semaphore_tests.c','semaphore_irq.h','semaphore_irq.c','semaphore_m4.c','SEMAPHORE.md','run_semaphore.py')]
    public += ['labs/concurrency_lifecycle_depth/wait/'+name for name in ('wait.h','wait.c','CONTRACTS.md')]
    public += ['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in public:
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record


def register():
    definitions=[
        ('COURSE','Actual course CMX/wait semaphore and original PR-T51-02 contract','curriculum/projects/PR-T51-02.md','course',
         ['assessor/execution/cmx.c semaphore take/give/close and actual waiter completion; labs/execution/include/cmx.h serialization/task contract',
          'assessor/concurrency_lifecycle_depth/wait.c enrollment/post/timeout/close and generation/count accounting; public wait contract',
          'fixtures/os_v3/PR-T51-02.json complete input/expected ledger; PR-T51-02 complete brief and T51.02 teaching/three original criteria'],
         'Bounded actual course-source reads; no external semaphore implementation is substituted.'),
        ('CMSIS','CMSIS-Core register access and NVIC priority policy','https://arm-software.github.io/CMSIS_6/main/Core/group__Core__Register__gr.html','primary',
         ['__get_BASEPRI/__set_BASEPRI nonzero threshold masking; __set_PRIMASK configurable-exception exclusion and NMI/HardFault exception',
          'https://arm-software.github.io/CMSIS_6/main/Core/group__NVIC__gr.html NVIC_SetPriority lower numbers more urgent/default0/implemented bits and NVIC_SetPendingIRQ'],
         'Selected actual current documentation read2026-10-17; the kernel threshold is a declared original port policy, not a universal RTOS API rule.'),
        ('REGISTERS','CMSIS6 Cortex-M4 SCB/NVIC register definitions','https://raw.githubusercontent.com/ARM-software/CMSIS_6/main/CMSIS/Core/Include/core_cm4.h','primary',
         ['SCB_Type ICSR offset0x004 and SHPR[12] offset0x018; SCB_ICSR_PENDSVSET bit28 and PENDSVCLR bit27',
          'NVIC ISER/ISPR/priority register layout and SCS/NVIC base definitions'],
         'Actual selected upstream header passages read2026-10-17, not an imported CMSIS runtime or full Arm manual inspection.')]
    rows=[]
    for suffix,title,url,kind,locators,limits in definitions:
        rows.append({'id':'R-LCSEM-'+suffix,'title':title,'authors':['C Mastery' if kind=='course' else 'Arm CMSIS authors'],
          'url':url,'kind':kind,'edition':'As identified in title and exact locator','domains':['T51'],'locators':locators,
          'access':'Actual bounded source/documentation passage inspected','role':'oracle','assignable':True,
          'reason':'Support exact permit ownership, saved-mask and deferred interrupt arguments.',
          'prerequisites':['PR-T51-01 and its generational CMX wait successor'],
          'inspection':{'date':'2026-10-17','method':'Read actual bounded source/manual passages before adapter/target construction.',
                        'scope':'; '.join(locators),'limits':limits}})
    write(BASE/'resources.json',{'schema_version':1,'resources':rows})
    write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T51'],
        'inspection_kind':'Author bounded primary/course-source inspection; no independent reviewer claim',
        'research_input_sha256':{'research/v5/expansion/legacy_concurrency_semaphore/resources.json':hashlib.sha256((BASE/'resources.json').read_bytes()).hexdigest()}})


def main():
    register();sys.path.insert(0,str(ROOT/'tools'))
    from check_semaphore_depth import validate_host,validate_target
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
                'implementation':{'state':'complete-for-declared-reference-and-changed-condition','evidence':['assessor/legacy_concurrency_sync/semaphore.c','assessor/legacy_concurrency_sync/semaphore_irq.c','assessor/execution/cmx.c','assessor/concurrency_lifecycle_depth/wait.c']},
                'author_validation':{'state':'passed-hosted-and-emulated-with-stated-scope','evidence':[HOST,TARGET]},
                'external_review':{'state':'not-run-for-this-successor','evidence':[]},
                'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-run-no-physical-claim','evidence':[]}},
            'variant_ids':[v['id'] for v in criterion['variants']],
            'required_environment':'Explicit completed CMX and wait imports; C17 serialized core; owned QEMU Cortex-M4 for actual IRQ0/PendSV transfer.',
            'external_blockers':['Independent review and actual delayed unaided learner attempts.','Physical target observation only for a physical claim.'],
            'evidence_limits':['No task context-switch, multicore scheduling, physical timing or certified execution-time bound is inferred.',
                               'Finite token histories and target schedules are declared subsets; no universal ordering/fairness or mastery proof.']}
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
          'research/v5/expansion/legacy_concurrency_semaphore/author_support.py','research/v5/expansion/legacy_concurrency_semaphore/resources.json',
          'tools/check_semaphore_depth.py']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
        'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
        'campaigns':[{'producer':'tools/run_semaphore_depth.py','report':HOST},{'producer':'tools/run_semaphore_target.py','report':TARGET}],
        'scope':'Only original T51.02 criteria and exact delayed variants; no whole-T51 or other project closure.',
        'remaining_author_work':[],'remaining_external_work':['Independent review, unaided delayed learner attempts and physical observations where claimed.']})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))


if __name__=='__main__':main()
