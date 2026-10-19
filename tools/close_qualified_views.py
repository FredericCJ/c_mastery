"""Attach and evidence-bind six retained qualified-view/lifetime tasks and transfers."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_qualified_views.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_qualified_views';PROJECT='PR-T02-03';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json'
SUPPORT='research/v5/expansion/legacy_qualified_views/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_qualified_views/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={'09.a': ['gcc-debug:0', 'gcc-debug:1', 'clang:target-O0'], '09.b': ['gcc-debug:0', 'clang:target-O3'], '09.c': ['gcc-debug:0', 'gcc:analysis-only', 'clang:analysis-only'], '10.a': ['gcc:qualification', 'clang:qualification', 'gcc-debug:0'], '10.b': ['gcc-debug:0', 'gcc-debug:2', 'clang-asan:0'], '10.c': ['gcc-debug:3', 'gcc-debug:oracle', 'clang-asan:oracle', 'gcc:volatile-4', 'clang:volatile-4', 'arm-none-eabi-gcc:target-O3', 'clang:target-O3']}
RATIONALE={'09.a': 'Actual initialization/assignment/reset member-value checks including pointer null; precise static-padding and representation analysis, original-helper closure audited.', '09.b': 'Actual designated immediate argument and modifiable file-scope literal/alias; explicit storage duration/lifetime reasoning.', '09.c': 'Direct and wrapped compound-literal escape specimens compile-only; actual caller-owner construction repair and view lifetime analysis.', '10.a': 'Actual three qualification levels, double-pointer/drop/opacity negative translations and positive control; opaque live getter with mutable owner alias.', '10.b': 'Actual capacity-atomic copy, live owner mutation/reset versus detached snapshot and wrong FULL publication mutant.', '10.c': 'Actual disjoint restricted copy plus overlap-safe successor,11979 independent snapshot cases and wrong-direction mutant; native/Thumb volatile access counts and cached-read code mutant, with thread/hardware limits.'}

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/qualified_views').iterdir()) if p.is_file()]
 put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T02',key='qualified_views_successor',supplied_artifacts=public,shared_contracts=['labs/qualified_views/CONTRACTS.md'],source_register='research/v5/expansion/legacy_qualified_views/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Six unchanged ordinary and six delayed tasks; actual strict-component records, qualification/lifetime and overlap/access investigations, no learner or physical claim'))
 x=attach(d);assert x['criteria']==old['criteria'] and attach(copy.deepcopy(x))==x;put(ROOT/CARD,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/qualified_views/CONTRACTS.md',learner_artifacts=['learner.c','member-value/lifetime/alias and qualifier dossier','criterion-indexed diagnosis and transfer dossier'],assessment='assessor/qualified_views/ANALYSIS.md#'+c['objective'].lower().replace('.',''),command_groups=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_qualified_views/resources.json') for c in d['criteria']],remaining='Other T02–06 and recursive-inventory tasks remain required.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['N1570 C11 draft is not published C17.','Hosted C17 qualified-view/lifetime experiments and explicit hypothetical integer models; no physical claim.']))
 print('PASS six original criteria retained; additive attachment idempotent')
def package():
 from check_qualified_views import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_qualified_views_package.py',ROOT/CARD}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json')
 for r in read(HERE/'resources.json')['resources']:
  for f,h in r['inspection']['source_sha256'].items():assert sha(ROOT/f)==h,('source changed',f);files.add(ROOT/f)
 d=read(ROOT/CARD);assert d['criteria']==original()['criteria'];rows=[]
 for c in d['criteria']:
  key=c['objective'][4:];v=c['variants'][0];groups=SELECTORS[key]
  for delayed in [False,True]:
   artifacts=['labs/qualified_views/CONTRACTS.md','assessor/qualified_views/ANALYSIS.md','research/v5/expansion/legacy_qualified_views/objective_mapping.json']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],command_groups=groups,analysis='assessor/qualified_views/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T02-03 six ordinary and six unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['Other foundations and recursive inventory authoring remains required.','External review, independent/delayed learner and physical evidence remain separate.'],summary=dict(criteria=6,delayed=6,commands=len(report['commands']),assertions=len(report['checks']),scope='Separate native semantics, negative translations, lifetime analysis, access-code inspection and strict runtime-closure evidence')));print('PASS12 exact qualified-view/lifetime dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
