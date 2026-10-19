"""Bind exact T52.09 criteria to placement, demand and actual native ownership evidence."""
from copy import deepcopy
import hashlib,json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_realtime_partition/support.json'
CONTRACT='labs/realtime_depth/partition/CONTRACTS.md';PROJECT='PR-T52-09'
CAMPAIGN='reviews/v5/expansion/legacy_realtime_partition/campaign.json'
SCOPES={
 'T52.09.a':'Original aggregate-utilization and input-order counterexamples, stable FFD identity preservation and constrained-deadline demand distinguish admissibility from density or global EDF claims.',
 'T52.09.b':'Transactional wrapper invokes actual retained cmx_partition for the homogeneous case; checked horizon/cost arithmetic and affinity/unequal-cost extensions preserve prior output, with exhaustive witnesses separating heuristic failure from impossibility.',
 'T52.09.c':'Independent1,038-case assignment/job-demand oracle, actual cmx_simulate per-CPU results, two native pinned workers and an explicit drained epoch-bound payload transfer separate finite certificates, native execution and physical timing.'}

def write(path,value):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')

def attach(record):
    if record['project']!=PROJECT:return record
    for criterion in record['criteria']:
        criterion['successor_support']={'path':SUPPORT,'id':criterion['id']}
    links=record.setdefault('additional_successor_support',[])
    links[:]=[r for r in links if r.get('path')!=SUPPORT]
    links.append({'path':SUPPORT,'criterion_ids':[r['id'] for r in record['criteria']],
      'scope':'Exact finite placement/demand/affinity, actual retained simulation and native pinned/transfer observations; no physical WCET, M7/M4 cost or hard-real-time execution guarantee.'})
    links.sort(key=lambda r:r['path'])
    if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
    public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/realtime_depth/partition').iterdir() if p.is_file()]
    public+=['labs/execution/include/cmx.h','labs/execution/include/cmx_advanced.h','labs/execution/src/advanced.c','labs/execution/src/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld']
    for path in sorted(public):
        if path not in record['supplied_artifacts']:record['supplied_artifacts'].append(path)
    return record

def main():
    sys.path.insert(0,str(ROOT/'tools'))
    from expansion_evidence import require_batch_research
    from check_partition_depth import validate_campaign
    require_batch_research(BASE)
    validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
    canonical=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json')
    record=attach(json.loads(canonical.read_text(encoding='utf-8')));write(canonical,record)
    updates=[]
    for criterion in record['criteria']:
        row={'id':criterion['id'],'project':PROJECT,'original_objective':criterion['objective'],
          'original_source':canonical.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[criterion['objective']],
          'dimensions':{
            'teaching':{'state':'complete-for-explicit-placement-demand-and-transfer-contract','evidence':[CONTRACT]},
            'implementation':{'state':'complete-for-original-finite-criteria-and-changed-conditions','evidence':['assessor/legacy_realtime/partition.c','labs/realtime_depth/partition/oracle.py','assessor/execution/advanced.c']},
            'author_validation':{'state':'passed-hosted-oracle-strict-M4-and-native-eight-profile-observations','evidence':[CAMPAIGN]},
            'external_review':{'state':'not-run','evidence':[]},
            'independent_learner':{'state':'not-assessed','evidence':[]},
            'physical_observation':{'state':'not-run-physical-WCET-and-heterogeneous-core-costs-not-claimed','evidence':[]}},
          'variant_ids':[v['id'] for v in criterion['variants']],
          'required_environment':'C17 finite externally serialized placement core with explicit completed CMX advanced producer; six hosted profiles, independent Python oracle, three strict M4 profiles and eight Linux native profiles including GCC/Clang ThreadSanitizer. Native harness needs two allowed CPUs.',
          'external_blockers':['Independent review and delayed unaided learner attempts remain separate.','Parent M7/M4 changed-target comparison needs actual selected-core cost and interference observations before physical claims.'],
          'evidence_limits':['Exact search proves feasibility or impossibility only for the encoded independent fully preemptible periodic constrained-deadline model and supplied cost tables.','The1,038 input sets include35 heuristic failures with an exact witness and497 accepted/simulated assignments; this is not every possible task set.','Native singleton-mask workers and drained400-step payload transfer are ordinary Linux execution, not hard-real-time scheduling or a physical WCET observation.','The controlled transfer requires a new destination admission/interference argument; the finite lease enforces ownership, not that timing proof.','Unequal cost-table support does not fabricate M7/M4 measurements; Cortex-M4 emulator runs separately establish executable/helper closure.']}

        updates.append(row)
        for variant in criterion['variants']:
            child=deepcopy(row)
            child.update(id=variant['id'],parent_id=criterion['id'],original_variant_id=variant['id'],
              original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],
              variant_readiness='Exact constrained-demand, affinity/exhaustive-witness or native drained-transfer variant implemented and author-executed; the independent seven-day learner attempt remains unassessed.')
            updates.append(child)
    gap=deepcopy(updates[0]);gap.pop('original_objective')
    gap.update(id='G-LCON-PR-T52-09',original_source='research/v5/expansion/legacy_concurrency/gap_ledger.json',
      closure_rationale='All three original placement criteria and exact changed conditions have bounded reference, oracle, actual producer and native transfer evidence; physical heterogeneous-cost/WCET claims and independent assessment remain explicit.',
      criterion_ids=[c['id'] for c in record['criteria']],variant_ids=[v['id'] for c in record['criteria'] for v in c['variants']])
    updates.append(gap)
    resource_path='research/v5/expansion/legacy_realtime_partition/resources.json'
    resources=json.loads((ROOT/resource_path).read_text(encoding='utf-8'))
    pins=[CONTRACT,CAMPAIGN,canonical.relative_to(ROOT).as_posix(),'tools/check_partition_depth.py',
      'assessor/legacy_realtime/partition.md','research/v5/expansion/legacy_realtime_partition/author_support.py',resource_path,
      'research/v5/expansion/legacy_realtime_partition/research_gate.json','research/v5/expansion/legacy_realtime_partition/source_notes.json']
    write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,
      'source_records':[{'id':r['id'],'path':resource_path,'passages':r['locators']} for r in resources['resources']],
      'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},
      'campaigns':[{'producer':'tools/run_partition_depth.py','report':CAMPAIGN}],
      'remaining_author_work':[],
      'remaining_external_work':['Independent review and delayed unaided learner assessment.','Actual selected M7/M4 cost/interference and physical WCET observations before asserting a physical scheduling guarantee.'],
      'scope':'Only T52.09 original criteria and exact changed conditions; separate finite placement/actual CMX/strict M4/native execution observations, with no closure of other realtime topics or physical/learner claims.'})
    print(json.dumps({'updates':len(updates),'criteria':3,'variants':3}))

if __name__=='__main__':main()
