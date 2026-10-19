"""Attach six exact model objectives and changed conditions; retain baseline."""
from copy import deepcopy
import hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
PROJECT='PR-T32-02';SUPPORT='research/v5/expansion/legacy_testing_models/support.json';CONTRACT='labs/testing_depth/models/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_testing_models/campaign.json'
SCOPES={
'T32.04.a':'Actual unchanged queue provider with independent list comparison of every0..6 word on push0/push1/pop, added clear transitions and32 versioned64-step seeded histories/profile.',
'T32.04.b':'Separate abstract/refusal/unused-storage properties plus revision-qualified actual slot borrows, stale same-slot reuse and finite-counter retirement; raw-pointer lifetime remains caller discipline.',
'T32.04.c':'Actual compiled storage-preservation failure reduced with exact class, complete seed/version/trace/binary identities; six actual controlled pthread schedules and schedule-preserving reduction retain lost-update, not unrelated crashes.',
'T32.05.a':'Two actual staged byte decoders with different representations reconcile count/extent/endian/error precedence; noncanonical high-bit count alias explicitly classified outside canonical common domain.',
'T32.05.b':'Exact half-unit FIR and independent Fraction convolution across343 inputs/four chunk splits/two saturation policies, original scaling values and saturation-at2 counterexample, carried history and invalid-domain preservation.',
'T32.05.c':'Actual C cursor/fold implementations share a deliberately wrong endian helper and two length checks share a wrong extent authority; independent hand-derived asymmetric six-byte wire vector falsifies agreement.'}
def write(p,v):p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(record):
 if record['project']!=PROJECT:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 rows=record.setdefault('additional_successor_support',[]);rows[:]=[r for r in rows if r.get('path')!=SUPPORT];rows.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'All six exact model/metamorphic criteria and changed conditions with actual provider, independent oracle, reduction and controlled native schedules; learner/review/physical dimensions separate.'});rows.sort(key=lambda r:r['path'])
 for p in (CONTRACT,'labs/testing_depth/architecture/CONTRACTS.md'):
  if p not in record['shared_contracts']:record['shared_contracts'].append(p)
 paths=['fixtures/embedded/PR-T32-02.json','labs/testing_depth/architecture/architecture.h','labs/testing_depth/architecture/architecture.c','labs/testing_depth/architecture/CONTRACTS.md',*sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/testing_depth/models').rglob('*') if p.is_file() and '__pycache__' not in p.parts)]
 for p in paths:
  if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
 return record
def main():
 sys.path.insert(0,str(ROOT/'tools'));from expansion_evidence import require_batch_research;from check_testing_models import validate_campaign
 require_batch_research(BASE);validate_campaign(json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8')))
 path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');record=attach(json.loads(path.read_text(encoding='utf-8')));write(path,record);updates=[]
 for c in record['criteria']:
  row={'id':c['id'],'project':PROJECT,'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':SCOPES[c['objective']],
   'dimensions':{'teaching':{'state':'complete-for-six-original-criteria-and-changed-conditions','evidence':[CONTRACT]},'implementation':{'state':'implemented-actual-provider-model-reducer-decoder-FIR-and-schedule-contracts','evidence':['assessor/testing_depth/models.c','assessor/testing_depth/models.md','labs/testing_depth/models/oracle.py','labs/testing_depth/models/schedule.c','labs/testing_depth/models/correlated.c']},'author_validation':{'state':'passed-six-profiles-independent-oracles-and-exact-class-controls','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-no-physical-target-inferred-from-hosted-models','evidence':[]}},
   'variant_ids':[v['id'] for v in c['variants']],'required_environment':'C17 GCC/Clang debug/release/ASan+UBSan on Linux; actual completed PR-T32-01 provider, Python Fraction/list oracle and original pthread controlled schedule target. No external network/hardware/host security changes.',
   'external_blockers':['Independent human review and original minimum-seven-day unaided learner assessment remain unattempted.','Physical target observations not established by native/model evidence.'],
   'evidence_limits':['Finite enumeration and selected controlled schedules do not prove every implementation execution.','Deletion reduction is1-deletion-minimal under the recorded semantic predicate, not a universal globally smallest reproducer.','Borrow revision cannot validate expired C object lifetime, enforce disjointness or permit concurrent use.','The two decoders share an explicit original specification; independent authority and correlated-error controls remain necessary.','Exact rational FIR evidence does not imply bitwise floating linearity, arbitrary coefficients or physical signal accuracy.','The schedule target uses actual pthreads with a controlled event order; it is not arbitrary stress, a memory-model proof or a physical timing result.','Published C17 remains unavailable; no external protocol-conformance claim.']}
  updates.append(row)
  for variant in c['variants']:
   child=deepcopy(row);child.update(id=variant['id'],parent_id=c['id'],original_variant_id=variant['id'],original_variant_task=variant['task'],minimum_days=variant['minimum_days'],variant_ids=[variant['id']],variant_readiness='Exact clear, borrow lifetime, schedule-preserving reduction, noncanonical reference, saturation and shared-length changed conditions implemented/executed; delayed learner evidence separate.');updates.append(child)
 resource=BASE/'resources.json';resources=json.loads(resource.read_text(encoding='utf-8'));pins=[CONTRACT,CAMPAIGN,path.relative_to(ROOT).as_posix(),'tools/check_testing_models.py','assessor/testing_depth/models.md',*[p.relative_to(ROOT).as_posix() for p in (BASE/'author_support.py',BASE/'register_sources.py',resource,BASE/'source_access.json',BASE/'research_gate.json')]]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':resource.relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']],'source_sha256':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_testing_models.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent review and delayed learner assessment.','Physical target evidence wherever subsequently claimed.'],'scope':'All six exact original model/metamorphic and changed-condition contracts, with unchanged actual queue producer and bounded native/model observations.'})
 print(json.dumps({'updates':len(updates),'criteria':6,'variants':6}))
if __name__=='__main__':main()
