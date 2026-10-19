"""Attach and evidence-bind nine retained variadic/expression tasks and transfers."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_variadic_expressions.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_variadic_expressions';PROJECT='PR-T02-02';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json'
SUPPORT='research/v5/expansion/legacy_variadic_expressions/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_variadic_expressions/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={'06.a': ['gcc-debug:0', 'gcc:analysis-only'], '06.b': ['gcc-debug:0', 'gcc-debug:2', 'gcc:negative-translation'], '06.c': ['gcc-debug:0', 'gcc-debug:1', 'gcc-debug:3', 'clang-asan:oracle'], '07.a': ['gcc-debug:sequencing', 'clang-release:sequencing'], '07.b': ['gcc-debug:conversion', 'clang-release:conversion'], '07.c': ['gcc-debug:6', 'gcc-debug:oracle'], '08.a': ['gcc-debug:sequencing', 'gcc:analysis-only'], '08.b': ['clang-release:sequencing', 'clang:analysis-only'], '08.c': ['gcc-debug:4', 'gcc-debug:5', 'gcc-debug:oracle', 'clang-asan:oracle']}
RATIONALE={'06.a': 'Actual char/short/float/double calls, compile-only wrong request and exact promoted-type/exception analysis under changed integer ranges.', '06.b': 'Independent helper-owned va_copy lifetimes on every exit, actual capacity/conversion/allocation failures and release mutant; ABI assignment negative plus copy positive.', '06.c': 'Exact baseline tag protocol, capacity atomicity and all explicit-array admission/failure variants; independent literal-text cases.', '07.a': 'Actual six-permutation arithmetic trace admission, selected conditional branch and fixed full-expression temporaries with separate typed-tree analysis.', '07.b': 'Actual native comparison/type fixtures plus explicit int16/int32/int64 range-and-rank derivations; hypothetical models never relabeled target execution.', '07.c': 'Checked narrowing and lost-provenance analysis,2049 independent inputs and premature conversion mutant.', '08.a': 'Actual short-circuit/comma/body traces; changed distinct-versus-shared argument effects classified with undefined cases compile-only.', '08.b': 'Independent and shared body effects with allowed AB/BA result pairs; unsequenced argument modifications never executed.', '08.c': 'Actual ordered callbacks, gate/step failure partial progress, intermediate arithmetic admission,9261 independent equations and order/gate mutants.'}

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/variadic_expressions').iterdir()) if p.is_file()]
 put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T02',key='variadic_expressions_successor',supplied_artifacts=public,shared_contracts=['labs/variadic_expressions/CONTRACTS.md'],source_register='research/v5/expansion/legacy_variadic_expressions/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Nine unchanged ordinary and nine delayed tasks; actual variadic traversal, expression models and ordered callback failures, no learner or physical claim'))
 x=attach(d);assert x['criteria']==old['criteria'] and attach(copy.deepcopy(x))==x;put(ROOT/CARD,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/variadic_expressions/CONTRACTS.md',learner_artifacts=['learner.c','promotion ledger and typed-expression/sequencing notebook','criterion-indexed diagnosis and transfer dossier'],assessment='assessor/variadic_expressions/ANALYSIS.md#'+c['objective'].lower().replace('.',''),command_groups=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_variadic_expressions/resources.json') for c in d['criteria']],remaining='Other T02–06 and recursive-inventory tasks remain required.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['N1570 C11 draft is not published C17.','Hosted C17 variadic/expression experiments and explicit hypothetical integer models; no physical claim.']))
 print('PASS nine original criteria retained; additive attachment idempotent')
def package():
 from check_variadic_expressions import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_variadic_expressions_package.py',ROOT/CARD}
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
   artifacts=['labs/variadic_expressions/CONTRACTS.md','assessor/variadic_expressions/ANALYSIS.md','research/v5/expansion/legacy_variadic_expressions/objective_mapping.json']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],command_groups=groups,analysis='assessor/variadic_expressions/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T02-02 nine ordinary and nine unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['Other foundations and recursive inventory authoring remains required.','External review, independent/delayed learner and physical evidence remain separate.'],summary=dict(criteria=9,delayed=9,commands=len(report['commands']),assertions=len(report['checks']),scope='Separate native semantic, diagnostic/compile-only and hypothetical-model categories')));print('PASS18 exact variadic/expression dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
