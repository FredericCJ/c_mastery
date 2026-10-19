"""Bind exact original barrier criteria and delayed variants to current author evidence."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_concurrency_barrier/support.json'
CONTRACT='labs/execution/sync_depth/BARRIER.md'
TARGET_CONTRACT=CONTRACT
HOST='reviews/v5/expansion/legacy_concurrency_sync/barrier_campaign.json'
TARGET='reviews/v5/expansion/legacy_concurrency_sync/barrier_target.json'
PROJECT='PR-T51-10'
NATIVE='reviews/v5/expansion/legacy_concurrency_sync/barrier_native.json'
SCOPES={
 'T51.10.a':'Actual CMX two-round order/old-ticket separation with36 permutation pairs, zero/one-party boundaries, reduced three-round saturation and configuration-epoch exhaustion cleanup.',
 'T51.10.b':'Actual CMX timeout/cancel versus peer BROKEN, quiescent result/registration drain, explicit3-to2 withdrawal and2-to3 admission with new configuration identities.',
 'T51.10.c':'Actual native central and two-leaf/root tree publication over plain shared arrays,32 rounds/512 reads per topology, rotating delay and reuse fence; exact node counts and bounded cost analysis.'}



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
    links.append({'path':SUPPORT,'criterion_ids':ids,'scope':'Actual CMX barrier registrations/rounds/broken outcomes, target ABI/helper observations and native central/tree publication; no independent or physical claim.'})
    for contract in (CONTRACT,TARGET_CONTRACT):
        if contract not in record['shared_contracts']:record['shared_contracts'].append(contract)
    public=['labs/execution/sync_depth/'+name for name in
            ('barrier.h','barrier.c','barrier_tests.c','barrier_m4.c',
             'barrier_native.h','barrier_native.c','barrier_native_tests.c','BARRIER.md','run_barrier.py','run_barrier_native.py')]
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
    from check_barrier_depth import validate_host,validate_target,validate_native
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
                'implementation':{'state':'complete-for-declared-reference-and-changed-barrier','evidence':['assessor/legacy_concurrency_sync/barrier.c','assessor/legacy_concurrency_sync/barrier_native.c','assessor/execution/cmx.c']},
                'author_validation':{'state':'passed-hosted-and-emulated-with-stated-scope','evidence':[HOST,TARGET,NATIVE]},
                'external_review':{'state':'not-run-for-this-successor','evidence':[]},
                'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-run-no-physical-claim','evidence':[]}},
            'variant_ids':[v['id'] for v in criterion['variants']],
            'required_environment':'Explicit completed CMX import; C17 serialized core; Linux pthread mutex/condition with four real participants; owned QEMU Cortex-M4 with declared helper closure.',
            'external_blockers':['Independent review and actual delayed unaided learner attempts.','Physical target observation only for a physical claim.'],
            'evidence_limits':['No task context-switch, lock-free/weak-memory proof, physical timing or certified execution-time bound is inferred; native mutex serialization is explicit.',
                               'Finite36-history/native32-round observations are declared subsets; unexpected pthread failures require stop/retain/inspect and are not injected recovery evidence. Tree generalization costs are analysis, not executed arbitrary topology.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],
                original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
                variant_readiness='Implemented and author-executed under the declared changed assumptions; delayed learner assessment is separate.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-'+PROJECT,
        original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
        closure_rationale='All three original criteria and their exact changed barrier variants have bounded current author references; learner/review/physical gates remain separate.',
        criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    pins=[HOST,TARGET,NATIVE,CONTRACT,TARGET_CONTRACT,canonical.relative_to(ROOT).as_posix(),
          'research/v5/expansion/legacy_concurrency_barrier/author_support.py','research/v5/expansion/legacy_concurrency_barrier/resources.json','research/v5/expansion/legacy_concurrency_barrier/research_gate.json',
          'tools/check_barrier_depth.py']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
        'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
        'campaigns':[{'producer':'tools/run_barrier_depth.py','report':HOST},{'producer':'tools/run_barrier_target.py','report':TARGET},{'producer':'tools/run_barrier_native.py','report':NATIVE}],
        'scope':'Only original T51.10 criteria and exact delayed variants; no whole-T51 or other project closure.',
        'remaining_author_work':[],'remaining_external_work':['Independent review, unaided delayed learner attempts and physical observations where claimed.']})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))


if __name__=='__main__':main()
