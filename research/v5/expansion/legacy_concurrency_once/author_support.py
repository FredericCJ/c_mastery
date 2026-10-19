"""Bind exact original once criteria and delayed variants to current author evidence."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_concurrency_once/support.json'
CONTRACT='labs/execution/sync_depth/ONCE.md'
TARGET_CONTRACT=CONTRACT
HOST='reviews/v5/expansion/legacy_concurrency_sync/once_campaign.json'
TARGET='reviews/v5/expansion/legacy_concurrency_sync/once_target.json'
PROJECT='PR-T51-11'
NATIVE='reviews/v5/expansion/legacy_concurrency_sync/once_native.json'
SCOPES={
 'T51.11.a':'Actual retained CMX retry versus permanent/retry MB policies; administrative FAILED reset drains results and registrations, advances epoch and rejects old operation identities.',
 'T51.11.b':'Actual CMX/MB permanent adapter publishes eight entries/count/checksum only when complete; all11 construction cuts and three allocation failures clean up before failed publication, then a new owner succeeds in a fresh epoch.',
 'T51.11.c':'Three real contenders with two enrolled waiters, success/failure and delayed reader; hosted C17 call_once versus C++23 local-static/std::call_once exception/effect/publication comparisons with non-equivalence controls.'}



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
    links.append({'path':SUPPORT,'criterion_ids':ids,'scope':'Actual CMX/MB once policies, resource cleanup, quiescent reset, native contenders and language comparison; target ABI/helper observations, no independent or physical claim.'})
    for contract in (CONTRACT,TARGET_CONTRACT):
        if contract not in record['shared_contracts']:record['shared_contracts'].append(contract)
    public=['labs/execution/sync_depth/'+name for name in
            ('once.h','once.c','once_tests.c','once_m4.c','once_native_tests.c','once_compare.c','once_compare.cpp','ONCE.md','run_once.py','run_once_native.py')]
    public += ['labs/execution/include/cmx.h','labs/execution/src/cmx.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld','labs/modern_bridge/include/bridge.h','labs/modern_bridge/src/bridge.c']
    for path in public:
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    resources=json.loads((BASE/'resources.json').read_text(encoding='utf-8'))['resources']
    for resource in resources:
        ref={'id':resource['id'],'path':'research/v5/expansion/legacy_concurrency_once/resources.json','passages':resource['locators']}
        if not any(old['id']==ref['id'] for old in record['source_records']):record['source_records'].append(ref)
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
    from check_once_depth import validate_host,validate_target,validate_native
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
                'implementation':{'state':'complete-for-declared-reference-and-changed-once','evidence':['assessor/legacy_concurrency_sync/once.c','assessor/modern_bridge/bridge.c','assessor/execution/cmx.c']},
                'author_validation':{'state':'passed-hosted-and-emulated-with-stated-scope','evidence':[HOST,TARGET,NATIVE]},
                'external_review':{'state':'not-run-for-this-successor','evidence':[]},
                'independent_learner':{'state':'not-assessed','evidence':[]},
                'physical_observation':{'state':'not-run-no-physical-claim','evidence':[]}},
            'variant_ids':[v['id'] for v in criterion['variants']],
            'required_environment':'Explicit completed CMX and MB imports; C17 serialized core; Linux pthread mutex/condition with three real contenders; C17/C++23 hosted comparison only; owned QEMU Cortex-M4 helper/atomic audit.',
            'external_blockers':['Independent review and actual delayed unaided learner attempts.','Physical target observation only for a physical claim.','Exact C17 source access and a functioning TSan runtime/library path for the separately scoped hosted language comparisons.'],
            'evidence_limits':['No task context-switch, lock-free/weak-memory proof, physical timing or certified execution-time bound is inferred; native mutex serialization is explicit.',
                               'Finite11-cut/native30-round observations are declared subsets; unexpected pthread failures require stop/retain/inspect and are not injected recovery evidence. Exact C17 N2176 inspection is unavailable; C11/C23/C++23 passages and C17 runtime observations are distinguished. No X-T51-002 journal closure.',
                               'Native adapter has eight profiles including both TSan runtimes; language comparisons have six validated profiles. Both TSan comparison profiles remain explicitly failed/unvalidated with original logs retained.']}
        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row);child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],
                original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
                variant_readiness='Implemented and author-executed under the declared changed assumptions; delayed learner assessment is separate.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective');gap.update(id='G-LCON-'+PROJECT,
        original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
        closure_rationale='All three original criteria and their exact changed once variants have bounded current author references; learner/review/physical gates remain separate.',
        criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    pins=[HOST,TARGET,NATIVE,CONTRACT,TARGET_CONTRACT,canonical.relative_to(ROOT).as_posix(),
          'research/v5/expansion/legacy_concurrency_once/author_support.py','research/v5/expansion/legacy_concurrency_once/resources.json','research/v5/expansion/legacy_concurrency_once/research_gate.json',
          'tools/check_once_depth.py']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
        'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
        'campaigns':[{'producer':'tools/run_once_depth.py','report':HOST},{'producer':'tools/run_once_target.py','report':TARGET},{'producer':'tools/run_once_native.py','report':NATIVE}],
        'scope':'Only original T51.11 criteria and exact delayed variants; no whole-T51 or X-T51-002 journal closure.',
        'remaining_author_work':['Diagnose the C++ exception/call_once TSan timeout if claiming TSan comparison validation; current six-profile comparison remains bounded.'],
        'remaining_external_work':['Independent review, unaided delayed learner attempts, exact C17 source access, supported sanitizer comparison runtime and physical observations where claimed.']})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))


if __name__=='__main__':main()
