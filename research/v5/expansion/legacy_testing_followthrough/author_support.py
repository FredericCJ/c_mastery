"""Pure additive T32 attachments and exact source-bound criterion support."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(HERE))
from criterion_notes import NOTES
SUPPORT='research/v5/expansion/legacy_testing_followthrough/support.json';CAMPAIGN='reviews/v5/expansion/legacy_testing_followthrough/campaign.json';CONTRACT='labs/testing_followthrough/CONTRACTS.md';LAB=ROOT/'labs/testing_followthrough';REF=ROOT/'assessor/testing_followthrough';PROJECTS=[f'PR-T32-{i:02}' for i in range(4,8)]
def write(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf-8',newline='\n')
def records():return [json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(p+'.json')).read_text(encoding='utf-8')) for p in PROJECTS]
def public_paths():return sorted(p.relative_to(ROOT).as_posix() for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
def attach(record):
 if record['project'] not in PROJECTS:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 additions=record.setdefault('additional_successor_support',[]);additions[:]=[a for a in additions if a.get('path')!=SUPPORT];additions.append({'path':SUPPORT,'criterion_ids':[c['id'] for c in record['criteria']],'scope':'Exact original mechanism-specific transaction, diagnostic, static, coverage, mutation and replay obligations; independent and physical dimensions separate.'});additions.sort(key=lambda x:x['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for p in public_paths()+['labs/testing_depth/models/schedule.c']:
  if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
 return record
def prepare():
 rows=[];analysis=['# T32 assessor derivations','', 'Withheld from learner exports. Each original task and exact changed condition has a separate argument; tool/host/emulator/physical/learner authority remain separate.','']
 for record in records():
  for c in record['criteria']:
   base,variant,checks=NOTES[c['objective']]
   rows.append({'id':c['id'],'project':record['project'],'objective':c['objective'],'original_task':c['task'],'original_expected':c['verification']['expected'],'changed_conditions':copy.deepcopy(c['variants']),'teaching':CONTRACT,'mechanism_checks':checks,'learner_deliverables':['Prediction and explicit object/effect/environment contract.','Exact implementation or premise/counterexample analysis.','Raw discriminating positive/negative observations and source/tool identities.','Original changed-condition task after its minimum delay, unaided and separately observed.']})
   analysis+=['## '+c['id'],'',base,'','Changed condition: '+c['variants'][0]['task'],'',variant,'','Evidence selectors: '+', '.join(checks)+'.','']
 assert len(rows)==24 and set(NOTES)=={r['objective'] for r in rows}
 write(LAB/'criteria.json',{'schema_version':1,'criteria':rows,'scope':'Exact original tasks, teaching and check map; no assessor implementation or independent execution.'});(REF/'ANALYSIS.md').write_text('\n'.join(analysis)+'\n',encoding='utf-8',newline='\n');print('Prepared24 criteria and24 variants')
def main():
 sys.path.insert(0,str(ROOT/'tools'));from check_testing_followthrough import validate_campaign
 data=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(data)
 paths=[];updates=[]
 for record in records():
  p=ROOT/'curriculum/v5/expansion/legacy_assessments'/(record['project']+'.json');write(p,attach(record));paths.append(p)
  for c in record['criteria']:
   base,variant,checks=NOTES[c['objective']]
   row={'id':c['id'],'project':record['project'],'original_objective':c['objective'],'original_source':p.relative_to(ROOT).as_posix(),'closure_rationale':base,'mechanism_checks':checks,'variant_ids':[v['id'] for v in c['variants']],'dimensions':{'teaching':{'state':'complete-exact-original-and-changed-condition-contracts','evidence':[CONTRACT,'labs/testing_followthrough/criteria.json']},'implementation':{'state':'implemented-or-analyzed-exact-mechanism-and-changed-premise','evidence':['assessor/testing_followthrough/ANALYSIS.md','assessor/testing_followthrough/transaction.c','assessor/testing_followthrough/coverage.c','assessor/testing_followthrough/suppression.json',*public_paths()]},'author_validation':{'state':'passed-source-bound-mechanism-specific-host-tool-and-emulator-checks','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-board-routing-transport-baud-and-ADC-remain-open','evidence':[]}},'required_environment':'C17 GCC15.2/Clang21 Linux; distinct ASan/UBSan/TSan, plain Memcheck, Frama-C33 Eva, gcov condition/prime-path, GDB, two named QEMU Cortex-M3 machines. No host security setting changes.','external_blockers':['Independent human review and original minimum-seven-day unaided learner assessment.','Dedicated physical-board transport/routing and calibrated baud/ADC measurements where claimed.'],'evidence_limits':['Finite observed paths and explicit analyzer entry contexts only.','Controlled pthread schedules are not arbitrary concurrency or weak-memory proof.','Overheads are workload/build-specific; no actual allocator timing failure is invented.','Tool upgrades and reviewer challenges are worked invalidation analyses, not claimed future/independent observations.','Two QEMU core observations do not establish physical board behavior.','Exact published C17 source remains inaccessible.']}
   updates.append(row)
   for v in c['variants']:
    child=copy.deepcopy(row);child.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],closure_rationale=variant,variant_ids=[v['id']],variant_readiness='Exact changed-condition implementation or worked premise/counterexample analysis; independent delayed learner execution remains separate.');updates.append(child)
 resources=json.loads((HERE/'resources.json').read_text());pins=[*paths,ROOT/CAMPAIGN,Path(__file__),HERE/'criterion_notes.py',HERE/'register_sources.py',HERE/'resources.json',HERE/'source_access.json',HERE/'research_gate.json',ROOT/'tools/check_testing_followthrough.py',*[ROOT/p for p in data['inspected_paths']]]
 write(ROOT/SUPPORT,{'schema_version':1,'required':True,'updates':updates,'source_records':[{'id':r['id'],'path':(HERE/'resources.json').relative_to(ROOT).as_posix(),'passages':r['locators']} for r in resources['resources']],'source_sha256':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in pins},'campaigns':[{'producer':'tools/run_testing_followthrough.py','report':CAMPAIGN}],'remaining_author_work':[],'remaining_external_work':['Independent review and delayed learner assessment.','Dedicated-board and calibrated physical observations.'],'scope':'All24 exact original T32.07..14 criteria and24 delayed variants, with distinct mechanism-specific observations or worked premise analyses.'});print(json.dumps({'criteria':24,'updates':len(updates)}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');a=p.parse_args();prepare() if a.prepare else main()
