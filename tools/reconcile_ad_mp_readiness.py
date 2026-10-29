"""Preserve original obligations while deriving current author/external facets."""
import json
from pathlib import Path
from common import ROOT,dump,read_json,sha
from run_adt_measurement_closure import NAMES,FOLDERS,CONTRACTS,REPORT
from check_adt_measurement_closure import check
from run_metaprog_measurement_closure import REPORT as MP_REPORT
from check_metaprog_measurement_closure import check as check_mp
def main():
    d=read_json(ROOT/REPORT);check(d)
    limitations={
     'owning_sum':'Three-byte construct/copy/drop includes allocator and observation work; no relative zero-cost claim.',
     'rational_units':'Raw scale and wrappers have the same signed rational/precision/refusal contract and values.',
     'flags_negotiation':'Single negotiation workload; no comparison to a different feature-admission policy.',
     'utf8_stream':'Eight one-byte feeds per stream; prefix copying and transactional whole-chunk refusal retained.',
     'relocating_pool':'Both copied-get series return the same value in serialized use; raw baseline does not have the successor concurrent lifetime contract.',
     'mutable_views':'Scalar/vector traverse the same padded layout with checked widened sum, repeated-read semantics and identical checksum.',
     'arena_flow':'Cached-query series includes certificate construction and requires input immutability; worklist series is separate work.',
     'partial_optics':'Direct/composed replacements share a domain but composed adds admission; callback series has additional observable work.',
     'evolution_record':'Canonical relay and full authentication are distinct workloads, not a relative-overhead comparison.',
     'packet_ring':'Borrow and copied payload series retain the same release/cancellation point; copying does not free capacity early.'}
    matrix=[]
    dimensions=['Representation','Reachable storage','Lifetime','Mutation/aliasing','Allocation','Movement/copying','Destruction','Dispatch','Concurrency','Failure','Compilation','Binary/stack','Boundary']
    for i,(name,folder,contract) in enumerate(zip(NAMES,FOLDERS,CONTRACTS),1):
        ready='research/v5/modern/adt_'+name+'_readiness.json';r=read_json(ROOT/ready);primary=read_json(ROOT/list(r['current_reports'])[0]);metrics=[x for x in primary['observations'] if x['kind']=='metrics']
        matrix.append(dict(exercise=f'X-T67-{i:03}',rubrics=[x['id'] for x in r['criteria']],contract='labs/adt/'+folder+'/'+contract,dimensions=dimensions,readiness=ready,current_reports=r['current_reports'],observed_metrics=metrics,equivalence_limit=limitations[name],measurement_claim='Local raw samples and operation/resource counts under the named exact contract; no universal performance, learner or physical conclusion.'))
    matrix_path='research/v5/modern/adt_measurement_readiness.json';inputs=dict(d['input_sha256'])
    for p in [REPORT,'tools/check_adt_measurement_closure.py','tools/reconcile_ad_mp_readiness.py']:inputs[p]=sha(ROOT/p)
    dump(ROOT/matrix_path,dict(schema_version=1,id='T67-MEASUREMENTS',rows=matrix,input_sha256=inputs,current_reports={REPORT:sha(ROOT/REPORT)},author_readiness='ready-bounded',independent_learner='not-assessed',external_review='not-run',reference_limits=['Published C17/C23 not inspected','Installed mdspan headers unavailable','Haskell comparison inspected, not executed'],limits=d['limits']))
    md=read_json(ROOT/MP_REPORT);check_mp(md);mp_matrix_path='research/v5/modern/metaprog_measurement_readiness.json';mp_inputs=dict(md['input_sha256'])
    for p in [MP_REPORT,'tools/check_metaprog_measurement_closure.py','tools/reconcile_ad_mp_readiness.py']:mp_inputs[p]=sha(ROOT/p)
    dump(ROOT/mp_matrix_path,dict(schema_version=1,id='T68-MEASUREMENTS',rows=md['packets'],input_sha256=mp_inputs,current_reports={MP_REPORT:sha(ROOT/MP_REPORT)},author_readiness='ready-bounded',independent_learner='not-assessed',external_review='not-run',physical_observation='not-run',limits=md['limits']))
    original='research/v5/modern/ad_mp_gap_ledger.json';g=read_json(ROOT/original)
    completed={}
    for name in NAMES:
        p='research/v5/modern/adt_'+name+'_readiness.json';r=read_json(ROOT/p);index=int(r['criteria'][0]['id'].split('-')[2]);completed[f'T67-{index:02}']=p
    # Exact T68 packets appear only after their own current guard/readiness exists.
    import importlib
    for path in sorted((ROOT/'research/v5/modern').glob('metaprog_*_readiness.json')):
        r=read_json(path);p=path.relative_to(ROOT).as_posix()
        if r.get('domain')!='T68' or not r.get('criteria'):continue
        primary=next(iter(r['current_reports']));name=Path(primary).name.removesuffix('_validation.json')
        module=importlib.import_module('check_'+name)
        module.check(read_json(ROOT/module.REPORT),read_json(ROOT/module.COST))
        index=int(r['criteria'][0]['id'].split('-')[2]);completed[f'T68-{index:02}']=p
    author_open=[];external=[]
    for row in g['gaps']:
        prefix=row['id'].rsplit('-',1)[0]
        if row['id']=='T67-MEASUREMENTS':
            row.update(status='author-support-implemented-and-source-validated; external-assessment-not-run',author_evidence=[matrix_path,REPORT],resolved_descendants=[r['exercise'] for r in matrix],dimensions=dict(teaching='complete',implementation='complete',author_validation='passed',external_review='not-run',independent_learner='not-assessed',physical_observation='not-applicable-host-contract'));external.append(row['id'])
        elif row['id']=='T68-MEASUREMENTS':
            row.update(status='author-support-implemented-and-source-validated; external-assessment-not-run',author_evidence=[mp_matrix_path,MP_REPORT],resolved_descendants=[r['exercise'] for r in md['packets']],dimensions=dict(teaching='complete',implementation='complete',author_validation='passed',external_review='not-run',independent_learner='not-assessed',physical_observation='not-run'));external.append(row['id'])
        elif prefix in completed:
            p=completed[prefix];r=read_json(ROOT/p);row.update(status='author-support-implemented-and-source-validated; independent-learner-not-assessed',author_evidence=[p,*r['current_reports']],resolved_descendants=[x['id'] for x in r['criteria']],dimensions=dict(teaching='complete',implementation='complete',author_validation='passed',external_review='not-run',independent_learner='not-assessed',physical_observation=r['criteria'][0].get('physical_observation','not-applicable-host-contract')));external.append(row['id'])
        elif row['kind']=='unavailable-reference':row['dimensions']=dict(reference_access='unavailable');external.append(row['id'])
        else:author_open.append(row['id']);row['dimensions']=dict(teaching='authored',implementation='required-unfinished',author_validation='required-unfinished',external_review='not-run',independent_learner='not-assessed',physical_observation='unadjudicated')
        row['original_task_preserved']=True
    g.update(required_open=len(g['gaps']),required_author_work=author_open,required_external_evidence=external,comprehensive_complete=False,count_semantics='required_open retains each original obligation with an unfinished author, access, learner or review facet; author-complete is not accepted mastery.');dump(ROOT/original,g)
    remaining=dict(domains=['T67','T68'],authoring_status='Source-derived per-objective author support; original obligations retained with explicit facets',learner_status='not-assessed',required_open_gaps=[r['id'] for r in g['gaps']],required_author_work=author_open,required_external_evidence=external,remaining_domains=sorted({x.split('-')[0] for x in author_open}),scope='Only T67/T68. Other modern domain status belongs to its own source-bound records; no stale T76 global claim.',preserve_prior_gap_ledgers=True,comprehensive_complete=False);dump(ROOT/'research/v5/modern/ad_mp_remaining_work.json',remaining)
    audit_path=ROOT/'research/v5/modern/ad_mp_depth_audit.json';a=read_json(audit_path);a['current_successor_support']=dict(ready_packets=len(completed),explicit_rubrics=4*len(completed),measurement_matrix=matrix_path,measurement_matrices=[matrix_path,mp_matrix_path],remaining_author_obligations=author_open,claim='Author implementation/observations only; original baseline and external requirements remain.');dump(audit_path,a)
    inputs[matrix_path]=sha(ROOT/matrix_path)
    update=dict(id='T67-MEASUREMENTS',dimensions={'teaching':dict(state='complete',evidence=[r['contract'] for r in matrix]),'implementation':dict(state='complete',evidence=[r['readiness'] for r in matrix]),'author_validation':dict(state='passed',evidence=[REPORT,matrix_path]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-applicable-host-contract',evidence=['curriculum/v5/exercises/modern-cost-contract.md'])},closure_rationale='All ten actual per-mechanism packets are freshly guarded with their exact40 rubric bindings. The matrix retains all13 cost dimensions, actual metrics/object/stack observations and fills first4 clean/unchanged build measurements. Non-equivalent pool/cached/callback/authentication contracts are explicit; no universal zero-overhead or physical claim. Original rows and external evidence remain.')
    dump(ROOT/'research/v5/modern/adt_measurement_completion_dispositions.json',dict(schema_version=1,updates=[update],input_sha256=inputs,current_reports=[REPORT]))
    mp_inputs[mp_matrix_path]=sha(ROOT/mp_matrix_path)
    mp_update=dict(id='T68-MEASUREMENTS',dimensions={'teaching':dict(state='complete',evidence=[r['contract'] for r in md['packets']]),'implementation':dict(state='complete',evidence=[r['readiness'] for r in md['packets']]),'author_validation':dict(state='passed',evidence=[MP_REPORT,mp_matrix_path]),'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),'physical_observation':dict(state='not-run',evidence=['curriculum/v5/exercises/modern-cost-contract.md'])},closure_rationale='Ten current per-mechanism guards bind40exact rubrics, all13cost dimensions, six-profile raw struct lr_cpu samples/object/stack observations and actual clean/unchanged builds. Derived mean iteration costs/rates preserve raw values and zero-resolution limits; only listed same-contract groups are compared. No physical bus/power-loss, tail-latency, universal zero-overhead or learner claim. Original rows and access/external evidence remain.')
    dump(ROOT/'research/v5/modern/metaprog_measurement_completion_dispositions.json',dict(schema_version=1,updates=[mp_update],input_sha256=mp_inputs,current_reports=[MP_REPORT]));print(json.dumps(dict(passed=True,ready_packets=len(completed),remaining_author=len(author_open),original_rows=len(g['gaps']))))
if __name__=='__main__':main()
