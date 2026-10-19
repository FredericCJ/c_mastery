"""Attach and evidence-bind nine retained list/workflow tasks and nine transfers."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_foundation_list.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_foundation_list';PROJECT='PR-T01-03';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json'
SUPPORT='research/v5/expansion/legacy_foundation_list/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_foundation_list/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={'04.a':['T01.04-stages'],'04.b':['T01.04-frame'],'04.c':['T01.04-stages'],'05.a':['T01.05-shell'],'05.b':['T01.05-revisions'],'05.c':['identities','T01.05-revisions'],'06.a':['gcc-debug:0','gcc-debug:2','gcc-debug:3','gcc-debug:5','capacity-transfer'],'06.b':['gcc-debug:oracle','gcc-debug:4','clang-asan:oracle'],'06.c':['T01.06-diagnosis','gcc-debug:1']}
RATIONALE={'04.a':'Actual two-TU stages and explicit input-language selection with no-suffix control.','04.b':'Actual O0/O3 x86-64 object inspection, parameter/return ABI and local storage analysis.','04.c':'Distinct actual translation/link/interpreter/runtime failures and owned shared-library move/restore.','05.a':'Actual fresh-environment shell and spaced source/output path replay.','05.b':'Actual clean defect/repair worktrees and repair rebased onto renamed API, old/new caller outcomes.','05.c':'Source/tool/target/argv/raw-result identities and actual compiler/optimization replay.','06.a':'Original bounded grammar, atomic numeric output, incremental byte/EOF state, actual semantic controls.','06.b':'Independent whole-token/bignum oracle, explicit partitions and collect-all diagnostic ownership.','06.c':'Separate analysis-only signed UB, defined-range deletion minimization and actual stale-object causal experiment.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/foundation_list').iterdir()) if p.is_file()]
 put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T01',key='foundation_list_successor',supplied_artifacts=public,shared_contracts=['labs/foundation_list/CONTRACTS.md'],source_register='research/v5/expansion/legacy_foundation_list/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Nine unchanged ordinary and nine delayed tasks; real original local compiler/loader/Git/list experiments, no learner or physical claim'))
 x=attach(d);assert x['criteria']==old['criteria'] and attach(copy.deepcopy(x))==x;put(ROOT/CARD,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/foundation_list/CONTRACTS.md',learner_artifacts=['learner.c','two-TU build/revision manifest','criterion-indexed derivation and diagnosis dossier'],assessment='assessor/foundation_list/ANALYSIS.md#'+c['objective'].lower().replace('.',''),command_groups=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_foundation_list/resources.json') for c in d['criteria']],remaining='Other T01–06 and recursive-inventory tasks remain required.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['N1570 C11 draft is not published C17.','Hosted Linux x86-64 ABI/OS experiments; no strict freestanding or physical claim.']))
 print('PASS nine original criteria retained; additive attachment idempotent')
def package():
 from check_foundation_list import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_foundation_list_package.py',ROOT/CARD}
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
   artifacts=['labs/foundation_list/CONTRACTS.md','assessor/foundation_list/ANALYSIS.md','assessor/foundation_list/DELAYED.md','research/v5/expansion/legacy_foundation_list/objective_mapping.json']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],command_groups=groups,observation=dict(objective=c['objective'],variant=delayed),analysis='assessor/foundation_list/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T01-03 nine ordinary and nine unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['Other foundations and recursive inventory authoring remains required.','External review, independent/delayed learner and physical evidence remain separate.'],summary=dict(criteria=9,delayed=9,commands=len(report['commands']),assertions=len(report['checks']),observations=len(report['observations']))));print('PASS18 exact list/workflow dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
