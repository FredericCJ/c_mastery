"""Integrate and evidence-bind the twelve exact original entry assessment tasks."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_foundation_entry.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_foundation_entry'
SUPPORT='research/v5/expansion/legacy_foundation_entry/completion_dispositions.json'
PROJECT='PR-T01-01';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json'
REVIEW='reviews/v5/expansion/legacy_foundation_entry/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={'01.a':['gcc-debug 0 execute','gcc-debug 1 execute'],'01.b':['gcc-debug 0 execute','gcc-debug 2 execute','gcc-debug 3 execute'],'01.c':['gcc-debug 0 execute','gcc-debug 4 execute'],'02.a':['gcc-debug 0 execute','gcc-debug 5 execute'],'02.b':['gcc-debug 0 execute','gcc-debug 6 execute'],'02.c':['gcc-debug 0 execute','original synthetic fixtures']}
RATIONALE={'01.a':'Actual recursive and bounded explicit stack traces, including base activation and partial failure; independent direct-formula suffix cases.','01.b':'Actual original fixed prefix plus two-pass and allocated-stage successors with capacity17, allocation/refusal/rollback and overlap; distinct resource and aliasing arguments.','01.c':'Checked-before-add invariant and unchanged-output rejection; implemented total empty/singleton/negative maximum transfer.','02.a':'Weighted binary/hex/two-octet derivation and implemented three-octet wire codec with capacity/range refusal and endian mutant.','02.b':'Exact any/all/forbidden contracts and65536 independently enumerated per-bit mask cases per reference profile.','02.c':'Original signed synthetic cases, checked bounded arithmetic, defined unsigned wrap and nonassociative saturation, without executing signed-overflow golden cases.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/foundation_entry').iterdir()) if p.is_file()]+['labs/foundations/include/foundation.h']
 put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T01',key='foundation_entry_successor',supplied_artifacts=public,shared_contracts=['labs/foundation_entry/CONTRACTS.md'],source_register='research/v5/expansion/legacy_foundation_entry/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Six exact original entry criteria and their six changed-condition implementations; no independent/delayed learner or physical claim'))
 x=attach(d);assert x['criteria']==old['criteria'] and attach(copy.deepcopy(x))==x;put(ROOT/CARD,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/foundation_entry/CONTRACTS.md',learner_artifacts=['learner.c','explicitly supplied original f_prefix','criterion-indexed trace/proof/cost/representation dossier'],assessment='assessor/foundation_entry/ANALYSIS.md#'+c['objective'].lower().replace('.',''),checks=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_foundation_entry/resources.json') for c in d['criteria']],remaining='Other T01–06 tasks and the recursive inventory remain required; this closes no whole-domain claim.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['C11 committee draft passages are not exact published C17; no whole-book source inspection inferred.','Hosted and explicitly bounded M4 emulation evidence are separate from physical observation.']))
 print('PASS six original criteria retained; additive attachment idempotent')
def package():
 from check_foundation_entry import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_foundation_entry_package.py',ROOT/CARD}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json')
 for r in read(HERE/'resources.json')['resources']:
  for f,h in r['inspection']['source_sha256'].items():assert sha(ROOT/f)==h,('source changed',f);files.add(ROOT/f)
 d=read(ROOT/CARD);assert d['criteria']==original()['criteria'];rows=[]
 for c in d['criteria']:
  key=c['objective'][4:];v=c['variants'][0];labels=list(dict.fromkeys([*SELECTORS[key],'clang-release 0 execute','gcc-asan 0 execute','clang-asan 0 execute','arm-none-eabi-gcc-O0 execute','clang-O3 resources']))
  for delayed in [False,True]:
   artifacts=['labs/foundation_entry/CONTRACTS.md','assessor/foundation_entry/ANALYSIS.md','assessor/foundation_entry/DELAYED.md','research/v5/expansion/legacy_foundation_entry/objective_mapping.json']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/foundation_entry/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T01-01 six ordinary and six unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['Other T01–06 and recursive inventory authoring remains required.','External review, independent/delayed learners and exact published-standard access remain separate.'],summary=dict(criteria=6,delayed=6,checks=len(report['checks']))));print('PASS12 exact entry dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
