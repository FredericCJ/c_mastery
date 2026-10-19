"""Additive completion of the preserved original bounded fuzzing campaign."""
import copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent
SUPPORT='research/v5/expansion/legacy_testing_fuzzing/support.json'
CONTRACT='labs/testing_depth/fuzzing/CONTRACTS.md';CAMPAIGN='reviews/v5/expansion/legacy_testing_fuzzing/campaign.json'
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf-8',newline='\n')
def attach(r):
 if r['project']!='PR-T32-03':return r
 for c in r['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 a=r.setdefault('additional_successor_support',[]);a[:]=[x for x in a if x.get('path')!=SUPPORT];a.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in r['criteria']],'scope':'Preserved source-bound bounded parser campaign, exact stateful/optional/semantic variants; learner and physical evidence separate.'});a.sort(key=lambda x:x['path'])
 for p in (CONTRACT,'labs/testing_depth/models/CONTRACTS.md','labs/testing_depth/architecture/CONTRACTS.md'):
  if p not in r['shared_contracts']:r['shared_contracts'].append(p)
 paths=[p for p in (ROOT/'labs/testing_depth/fuzzing').rglob('*') if p.is_file() and '__pycache__' not in p.parts]
 paths += [ROOT/'labs/testing_depth'/d/f for d,fs in [('models',['models.h','models.c','CONTRACTS.md']),('architecture',['architecture.h','architecture.c','CONTRACTS.md'])] for f in fs]
 for p in paths:
  name=p.relative_to(ROOT).as_posix()
  if name not in r['supplied_artifacts']:r['supplied_artifacts'].append(name)
 return r
def main():
 sys.path.insert(0,str(ROOT/'tools'));from check_testing_fuzzing import validate_campaign;from run_testing_fuzzing import sources
 data=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(data)
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T32-03.json';r=attach(json.loads(path.read_text()));write(path,r)
 rows=[];scopes=['Exact opcode1/count/extent/reset grammar plus explicit epoch/session sequence and unchanged publication checks.','Original classified seeds, independent grammar replay and finite guided corpus; separate optional-tag grammar before expansion.','Preserved same-site read-bounds reports and deletion-minimal input with repaired controls; wrong-output semantic oracle separately discriminates.']
 for c,scope in zip(r['criteria'],scopes):
  row={'id':c['id'],'project':r['project'],'original_objective':c['objective'],'original_source':path.relative_to(ROOT).as_posix(),'closure_rationale':scope,'variant_ids':[v['id'] for v in c['variants']],'dimensions':{'teaching':{'state':'complete-exact-original-and-changed-condition-contracts','evidence':[CONTRACT]},'implementation':{'state':'implemented-original-local-targets-and-exact-variants','evidence':['assessor/testing_depth/fuzzing.c','assessor/testing_depth/fuzzing.md','labs/testing_depth/fuzzing/oracle.py']},'author_validation':{'state':'passed-preserved-source-bound-47-observation-campaign','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-host-only','evidence':[]}},'required_environment':'C17 GCC/Clang Linux, explicit completed queue/decoder imports, finite LLVM libFuzzer and isolated original sanitizer specimens.','external_blockers':['Independent review and delayed unaided learner assessment.'],'evidence_limits':['124 saved commands revalidated without rerunning diagnostic operations.','Finite observed paths only; no exploitability or physical result.','Generic interruption flag has no recoverable specific prohibited operation.']}
  rows.append(row)
  for v in c['variants']:
   child=copy.deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],variant_ids=[v['id']],variant_readiness='Implemented/executed author reference; delayed independent learner assessment separate.');rows.append(child)
 resources=json.loads((HERE/'resources.json').read_text())
 pins=[*sources(),path,ROOT/CAMPAIGN,Path(__file__),ROOT/'tools/check_testing_fuzzing.py',HERE/'resources.json',HERE/'source_access.json',HERE/'research_gate.json',HERE/'interruption_01.json']
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':rows,'source_records':[{'id':x['id'],'path':(HERE/'resources.json').relative_to(ROOT).as_posix(),'passages':x['locators']} for x in resources['resources']],'source_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_testing_fuzzing.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent review, delayed learner and any physical claims.'],'scope':'All three original fuzzing criteria and three variants; preserved raw interruption and campaigns.'})
 print(json.dumps({'criteria':3,'updates':len(rows)}))
if __name__=='__main__':main()
