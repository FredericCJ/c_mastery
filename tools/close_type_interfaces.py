"""Attach and evidence-bind fifteen retained type-interface tasks and fifteen transfers."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_type_interfaces.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_type_interfaces';PROJECT='PR-T02-01';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json'
SUPPORT='research/v5/expansion/legacy_type_interfaces/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_type_interfaces/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={'01.a': ['T02.01.a', 'gcc-debug:0'], '01.b': ['T02.01.b'], '01.c': ['T02.01.c', 'gcc-debug:0'], '02.a': ['T02.02.a', 'gcc-debug:1'], '02.b': ['T02.02.b'], '02.c': ['T02.02.c', 'gcc-debug:2', 'gcc-debug:oracle', 'clang-asan:oracle'], '03.a': ['T02.03.a'], '03.b': ['T02.03.b'], '03.c': ['T02.03.c'], '04.a': ['T02.04.a'], '04.b': ['T02.04.b'], '04.c': ['T02.04.c'], '05.a': ['T02.05.a', 'gcc-debug:0', 'gcc-debug:3'], '05.b': ['T02.05.b'], '05.c': ['T02.05.c', 'gcc-debug:4', 'clang-asan:0']}
RATIONALE={'01.a': 'Actual compatible row factory and distinct pointer-array negative; complete expression/lifetime derivation.', '01.b': 'Separate tentative/extern definitions, actual symbol identities and initialized-extern duplicate link refusal.', '01.c': 'Compatible full/aliased factory declarations and actual wrapper member calls with wrong-call translation control.', '02.a': 'Explicit type-category analysis, sizeof constraints and two independently compiled opaque consumers; distinct owner semantic control.', '02.b': 'Top-level versus pointee qualification in same/separate TUs, signed/unsigned callback incompatibility and checked actual wrapper; unsafe cast never run.', '02.c': 'Alias swap compiles, struct swap rejects; exact unit operations,112 independent equations and nonintegral truncation control.', '03.a': 'Actual scope-map, expired prototype parameter and prototype-only tag conflict with file-tag repair; scope-entry analysis.', '03.b': 'Positive namespace coexistence and actual ordinary-identifier typedef collision.', '03.c': 'Defined shadowing defect, direct repair and explicit context-captured status callback; actual warning observation.', '04.a': 'Separate object/symbol ownership and persistent local state; two identically named internal objects remain distinct.', '04.b': 'Visible/hidden extern analysis, register address constraint, actual joined worker TLS and precise C/POSIX enforcement boundary.', '04.c': 'Independent mismatched objects link but never execute; shared-header rejection/repair and actual GCC LTO negative/positive.', '05.a': 'Actual row operation admission/empty/callback contract; inspected C17/N3096 empty-list comparison with actual C23 capability controls.', '05.b': 'Actual adjusted-pointer/row sizeof and shape rejection; static minimum-access precondition analysis with valid control.', '05.c': 'Actual reordered callback adapter and retained serial job lifecycle/cancellation/reentry tests; false acknowledgment semantic mutant.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/type_interfaces').iterdir()) if p.is_file()]
 put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T02',key='type_interfaces_successor',supplied_artifacts=public,shared_contracts=['labs/type_interfaces/CONTRACTS.md'],source_register='research/v5/expansion/legacy_type_interfaces/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Fifteen unchanged ordinary and fifteen delayed tasks; actual typed interfaces, separate units and stateful callback experiments, no learner or physical claim'))
 x=attach(d);assert x['criteria']==old['criteria'] and attach(copy.deepcopy(x))==x;put(ROOT/CARD,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/type_interfaces/CONTRACTS.md',learner_artifacts=['learner.c','independent consumer and type/scope/linkage dossier','criterion-indexed derivation and diagnosis dossier'],assessment='assessor/type_interfaces/ANALYSIS.md#'+c['objective'].lower().replace('.',''),command_groups=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_type_interfaces/resources.json') for c in d['criteria']],remaining='Other T02–06 and recursive-inventory tasks remain required.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['N1570 C11 draft is not published C17.','Hosted C17 interface/TLS experiments; bounded N3096 draft comparison, no physical claim.']))
 print('PASS fifteen original criteria retained; additive attachment idempotent')
def package():
 from check_type_interfaces import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_type_interfaces_package.py',ROOT/CARD}
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
   artifacts=['labs/type_interfaces/CONTRACTS.md','assessor/type_interfaces/ANALYSIS.md','research/v5/expansion/legacy_type_interfaces/objective_mapping.json']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],command_groups=groups,analysis='assessor/type_interfaces/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T02-01 fifteen ordinary and fifteen unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['Other foundations and recursive inventory authoring remains required.','External review, independent/delayed learner and physical evidence remain separate.'],summary=dict(criteria=15,delayed=15,commands=len(report['commands']),assertions=len(report['checks']),scope='Separate native semantic, diagnostic/compile-only, link and TLS categories')));print('PASS30 exact type-interface dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
