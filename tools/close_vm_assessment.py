"""Add exact T46.12 successors without rewriting original assessment objects."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_vm_representation.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_vm_assessment';SUPPORT='research/v5/expansion/legacy_vm_assessment/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_vm_assessment/';PROJECTS=['PR-T46-12'];REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={
'12.a':['gcc-debug compiler build','gcc-debug vm build','gcc-debug chain record','gcc-debug chain controls','clang-release chain controls'],
'12.b':['gcc-debug diag original execute','gcc-debug diag verifier execute','gcc-debug diag roots execute','gcc-debug diag fuel execute','clang-asan diag original execute'],
'12.c':['gcc-debug capacity 16 execute','gcc-debug capacity 8 execute','gcc-debug async 0 execute','gcc-debug async 1 execute','gcc-debug async 2 execute','gcc-debug async 3 execute','gcc-debug create failure execute','clang-tsan async 0 execute']}
RATIONALE={
'12.a':'Actual source/compiler/verifier/image/VM/result chain with independent19-byte oracle, seven mixed-provenance controls, changed adapter42to43 and executable clean-export/replay procedure; actual independent reader remains pending.',
'12.b':'Actual original core and separately compiled verifier/root/fuel controls, exact distinguishing pairs and accepted-image/stale native loan boundary with defined synthetic wrong-owner observation.',
'12.c':'Explicit H8 capacity implementation and derived22versus14GC budget, actual file-backed asynchronous store/Vs/Vh composition with cancellation/reply-loss/join and creation failure; complete withheld seven-day package, learner attempt pending.'}



def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def project(p):return 'curriculum/v5/expansion/legacy_assessments/'+p+'.json'
def original(p):return json.loads(subprocess.check_output(['git','show','HEAD:'+project(p)],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/runtime_followthrough/assessment').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public+=['labs/runtime/include/cvm.h','labs/runtime/SPEC.md','labs/runtime/src/cvm.c','labs/storage/include/store.h','labs/storage/FORMAT.md','labs/storage/INTEGRATION.md','labs/runtime/adapters/store_host.h','labs/runtime/adapters/store_host.c','labs/runtime_followthrough/execution/execution.h','labs/runtime_followthrough/verifier/verifier.h','labs/runtime_followthrough/services/services.h']
 mapping=[]
 for name in PROJECTS:
  d=read(ROOT/project(name));old=original(name);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
  put(HERE/('attachment_'+name+'.json'),dict(project=name,domain='T46',key='vm_assessment_successor',supplied_artifacts=public,shared_contracts=['labs/runtime_followthrough/assessment/CONTRACTS.md','labs/runtime/SPEC.md'],source_register='research/v5/expansion/legacy_vm_assessment/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Actual provenance, semantic diagnosis, H8 capacity and file-backed asynchronous store variants; physical/external/learner dimensions separate'))
  x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/project(name),x)
  mapping.extend(dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/runtime_followthrough/assessment/CONTRACTS.md',learner_artifacts=['learner.c','original CVM and store','criterion-indexed provenance/diagnosis/capacity/continuation dossier'],assessment='assessor/runtime_followthrough/assessment/ANALYSIS.md#'+c['objective'][4:].replace('.',''),checks=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_vm_assessment/resources.json') for c in d['criteria'])
 put(HERE/'objective_mapping.json',dict(criteria=mapping,remaining='T46–49 and recursive inventory remain required; physical/source/external dimensions are separate.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['Named primary passages only; no final ISO wording or physical target result inferred.','Actual CVM1/compiler/Vs/Vh/store, hosted POSIX worker and explicitly separate H8 profile; hosted and target dependencies remain separate.']))
 print('PASS three original criteria preserved; additive attachments idempotent')
def package():
 from check_vm_assessment import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_vm_assessment_package.py',ROOT/'research/v5/expansion/legacy_vm_representation/author_support.py',*[ROOT/project(p) for p in PROJECTS]}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name not in {'completion_dispositions.json','exercise_completion_dispositions.json'});rows=[]
 for resource in read(HERE/'resources.json')['resources']:
  for path,digest in resource['inspection'].get('source_sha256',{}).items():assert sha(ROOT/path)==digest,('inspected source changed',path)
 for name in PROJECTS:
  d=read(ROOT/project(name));assert d['criteria']==original(name)['criteria']
  for c in d['criteria']:
   key=c['objective'][4:];v=c['variants'][0];labels=list(dict.fromkeys(SELECTORS[key]))
   for delayed in [False,True]:
    artifacts=['labs/runtime_followthrough/assessment/CONTRACTS.md','assessor/runtime_followthrough/assessment/ANALYSIS.md','assessor/runtime_followthrough/assessment/DELAYED.md','research/v5/expansion/legacy_vm_assessment/objective_mapping.json'];dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=name,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/runtime_followthrough/assessment/ANALYSIS.md#'+key.replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Physical target observations, exact published ISO access, independent learners/delayed and external review remain separate.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T46-12 three ordinary and three unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T46–49 and recursive inventory remain required authoring work.','Physical target observations, source gaps, external review and independent/delayed learner evidence remain separate.'],summary=dict(criteria=3,delayed=3,checks=len(report['checks']))));print('PASS6 exact VM assessment dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
