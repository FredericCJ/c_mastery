"""Attach and evidence-bind twelve retained integer-kernel tasks and transfers."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_integer_kernel.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_integer_kernel';PROJECT='PR-T04-01';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json'
SUPPORT='research/v5/expansion/legacy_integer_kernel/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_integer_kernel/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={'01.a': ['gcc:observations', 'clang:observations', 'arm-none-eabi-gcc:target-O0'], '01.b': ['gcc-debug:0', 'gcc:observations', 'clang:target-O3'], '01.c': ['gcc-debug:0', 'clang:target-O0'], '02.a': ['gcc:observations', 'clang:observations', 'gcc:analysis-only'], '02.b': ['gcc:observations', 'clang:observations', 'gcc-debug:3'], '02.c': ['gcc-debug:3', 'gcc-debug:4', 'arm-none-eabi-gcc:target-O3', 'clang:target-O3'], '03.a': ['gcc-debug:0', 'gcc-debug:5', 'gcc-debug:oracle'], '03.b': ['gcc-debug:oracle', 'gcc-debug:native', 'clang-asan:oracle', 'clang-asan:native', 'gcc-debug:1', 'gcc-debug:4', 'clang:target-O3'], '03.c': ['gcc:c17:False', 'gcc:c17:True', 'clang:c17:False', 'clang:c17:True', 'gcc:c23:False', 'gcc:c23:True', 'clang:c23:False', 'clang:c23:True'], '04.a': ['gcc-debug:0', 'gcc-debug:oracle', 'clang:analysis-only'], '04.b': ['gcc-debug:0', 'gcc-debug:native', 'arm-none-eabi-gcc:target-O0'], '04.c': ['gcc-debug:2', 'gcc-debug:oracle', 'clang-asan:native', 'clang:target-O3']}
RATIONALE={'01.a': 'Actual eleven-type limit/storage table and containing100000 decisions; separate explicit int16 literal/type model, no imaginary target run.', '01.b': 'Actual local pointer difference and integer logical base beyond PTRDIFF_MAX, negative input/wire-cell rejection; per-role representability and parser-boundary analysis.', '01.c': 'Actual octet/cell encode/decode with rejected cell256; explicit CHAR_BIT16 character-cell and packed-character transport derivation.', '02.a': 'Actual native promotions and bit-field observations; explicit int16/ushort model and representation premises; unsafe signed shift never executed.', '02.b': 'Actual negative-to-unsigned conversion and wider containing signed comparison; repair rejects negative before conversion.', '02.c': 'Actual native unavailable-source-value accounting and M4 input64/size32 SIZE_MAX/one-above executions; ceiling65535 and checked composed extent.', '03.a': 'Actual synthetic rejection, native130, unsigned modular example and separately clipped success/flag; clipped-flag mutant rejected.', '03.b': 'Manual same-width guards, all synthetic8-bit pairs plus symmetric/asymmetric ranges, independent quotient predicate search and Python native-edge oracle; actual object/image closure.', '03.c': 'Actual GCC/Clang builtins and capability-checked C23 stdckdint, staged failure publication, changed destination/infinite-precision comparison and actual unchanged foundation_depth import.', '04.a': 'Actual four-sign division, independent identity predicate oracle, and named inspected N617 older-rule proposal analysis without published-standard/runtime overclaim.', '04.b': 'Actual zero/asymmetric-edge rejection before operators and symmetric selected range success, native/target unchanged sentinels.', '04.c': 'Actual positive modulus without overflowing sum and negative-divisor FLOOR/EUCLIDEAN variants including INT_MIN divisor; independent expected outcomes and wrong-remainder mutant.'}


def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/integer_kernel').iterdir()) if p.is_file()]
 put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T04',key='integer_kernel_successor',supplied_artifacts=public,shared_contracts=['labs/integer_kernel/CONTRACTS.md'],source_register='research/v5/expansion/legacy_integer_kernel/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Twelve unchanged ordinary and twelve delayed tasks; exact integer mechanisms, models, independent oracles and actual target closure; no learner/physical claim'))
 x=attach(d);assert x['criteria']==old['criteria'] and attach(copy.deepcopy(x))==x;put(ROOT/CARD,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/integer_kernel/CONTRACTS.md',learner_artifacts=['learner.c','criterion-indexed integer type/guard/division dossier','criterion-indexed diagnosis and transfer dossier'],assessment='assessor/integer_kernel/ANALYSIS.md#'+c['objective'].lower().replace('.',''),command_groups=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_integer_kernel/resources.json') for c in d['criteria']],remaining='Other T02–06 and recursive-inventory tasks remain required.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['N1570 C11 draft is not published C17.','Hosted C17 integer-kernel experiments and explicit hypothetical integer models; no physical claim.']))
 print('PASS twelve original criteria retained; additive attachment idempotent')
def package():
 from check_integer_kernel import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_integer_kernel_package.py',ROOT/CARD}
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
   artifacts=['labs/integer_kernel/CONTRACTS.md','assessor/integer_kernel/ANALYSIS.md','research/v5/expansion/legacy_integer_kernel/objective_mapping.json']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],command_groups=groups,analysis='assessor/integer_kernel/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T04-01 twelve ordinary and twelve unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['Other foundations and recursive inventory authoring remains required.','External review, independent/delayed learner and physical evidence remain separate.'],summary=dict(criteria=12,delayed=12,commands=len(report['commands']),assertions=len(report['checks']),scope='Separate integer semantics, hypothetical models, independent oracles, compiler/edition comparisons and strict runtime closure')));print('PASS24 exact integer-kernel dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
