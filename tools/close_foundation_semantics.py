"""Integrate and evidence-bind the six exact original semantics assessment tasks."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_foundation_semantics.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_foundation_semantics'
SUPPORT='research/v5/expansion/legacy_foundation_semantics/completion_dispositions.json'
PROJECT='PR-T01-02';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json'
REVIEW='reviews/v5/expansion/legacy_foundation_semantics/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={'03.a':['gcc-debug 0 execute','gcc-debug 1 execute','gcc-debug 2 execute','gcc wrong_row constraint','clang wrong_matrix constraint'],'03.b':['gcc-debug 0 execute','gcc-debug 3 execute','gcc O0 analysis-only','clang O3 analysis-only'],'03.c':['gcc-debug 0 execute','gcc-debug 4 execute','gcc-debug 5 execute','gcc O0 analysis-only']}
RATIONALE={'03.a':'Actual pointer-plus-length, row-pointer and matrix interfaces;117 metadata cases, exact object/stride reasoning, separate negative translation and defined wrong-stride/admission controls.','03.b':'Analysis-only faulty bounds/initialization; executable complete branch repair and policy mutant with both branches and failure preservation; warnings recorded as tool observations.','03.c':'Actual independent function-body effects and comma operator; separate event-graph analysis of unsequenced argument modifications; undefined specimens never executed.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/foundation_semantics').iterdir()) if p.is_file()]
 put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T01',key='foundation_semantics_successor',supplied_artifacts=public,shared_contracts=['labs/foundation_semantics/CONTRACTS.md'],source_register='research/v5/expansion/legacy_foundation_semantics/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three exact original semantic criteria and their three changed-condition implementations; no independent/delayed learner or physical claim'))
 x=attach(d);assert x['criteria']==old['criteria'] and attach(copy.deepcopy(x))==x;put(ROOT/CARD,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/foundation_semantics/CONTRACTS.md',learner_artifacts=['learner.c','criterion-indexed pointer/init/event-graph dossier'],assessment='assessor/foundation_semantics/ANALYSIS.md#'+c['objective'].lower().replace('.',''),checks=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_foundation_semantics/resources.json') for c in d['criteria']],remaining='Other T01–06 tasks and the recursive inventory remain required; this closes no whole-domain claim.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['C11 committee draft passages are not exact published C17; no whole-book source inspection inferred.','Hosted profile only; no physical or freestanding execution claimed.']))
 print('PASS three original criteria retained; additive attachment idempotent')
def package():
 from check_foundation_semantics import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_foundation_semantics_package.py',ROOT/CARD}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json')
 for r in read(HERE/'resources.json')['resources']:
  for f,h in r['inspection']['source_sha256'].items():assert sha(ROOT/f)==h,('source changed',f);files.add(ROOT/f)
 d=read(ROOT/CARD);assert d['criteria']==original()['criteria'];rows=[]
 for c in d['criteria']:
  key=c['objective'][4:];v=c['variants'][0];labels=list(dict.fromkeys([*SELECTORS[key],'clang-release 0 execute','gcc-asan 0 execute','clang-asan 0 execute']))
  for delayed in [False,True]:
   artifacts=['labs/foundation_semantics/CONTRACTS.md','assessor/foundation_semantics/ANALYSIS.md','research/v5/expansion/legacy_foundation_semantics/objective_mapping.json']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/foundation_semantics/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T01-02 three ordinary and three unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['Other T01–06 and recursive inventory authoring remains required.','External review, independent/delayed learners and exact published-standard access remain separate.'],summary=dict(criteria=3,delayed=3,checks=len(report['checks']))));print('PASS6 exact semantics dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
