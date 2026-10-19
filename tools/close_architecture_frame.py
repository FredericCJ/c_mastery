"""Bind exact T37.09 original and delayed tasks to actual composed execution."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_frame'
from research.v5.expansion.legacy_architecture_frame.author_support import attach
PROJECT='PR-T37-07';RECEIPT='research/v5/expansion/legacy_architecture_frame/validation.json';SUPPORT='research/v5/expansion/legacy_architecture_frame/completion_dispositions.json'
ARTIFACTS=['labs/architecture_frame/CONTRACTS.md','assessor/architecture_frame/ANALYSIS.md','assessor/architecture_frame/ledger.c','assessor/architecture_frame/oracle.py','labs/architecture_frame/tests.c','labs/architecture_frame/target_build.py','assessor/architecture_frame/stack.py','assessor/architecture_frame/vectors.py','assessor/interrupt_depth/frame/frame.c','assessor/interrupt_depth/frame/target.c','assessor/interrupt_depth/frame/target.S']
RATIONALES={'T37.09.a':'Original reset/eight-word fixture and strict no-FP/no-padding policy actually compose Xf; inspected extension admits padded/FP/lazy variants with preserved outputs and architectural execution.','T37.09.b':'Explicit hardware/AAPCS ownership masks and two-context LR/SP restoration compose actual priority groups; actual unchanged handler captures nested MSP versus PSP with FP-callee changed assumption.','T37.09.c':'Actual sixteen-row MPS2 observations, linked vector bytes and stack/closure audits are separate from Flash/SRAM reset alias model; changed boot mapping identifies renewed physical evidence without claiming it observed.'}
REMAINING=['PR-T37-08 original and changed performance mechanisms remain separately required.','Real F439 board/boot/option-byte/errata observations, physical latency, external review, independent learner/delay and final published C17/C23 remain separate.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T37.09.a':['.reset','.capture','.mutation.RESET','.mutation.IRQ','.mutation.POLICY','.mutation.FP_','.O0.','.O3.','thumb.'],'T37.09.b':['.ownership','.nested','.mutation.LR','.mutation.R4','.mutation.STACK','.O0.','.O3.','thumb.'],'T37.09.c':['.reset','.capture','.O0.','.O3.','thumb.']};return dict(report=RECEIPT,checks=[n for n in commands if any(p in n for p in patterns[objective])])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def all_rows(r):
 rows=[];commands=[q['id'] for q in r['commands']]
 for c in read(canon())['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-frame.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/architecture_frame').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/interrupt_depth/frame/'+n for n in ['frame.h','frame.c','target.c','target.S','target.ld','CONTRACTS.md']]+['labs/containers_depth/byte_helpers.c']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T37',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/interrupt_depth/frame/CONTRACTS.md'],source_register='research/v5/expansion/legacy_architecture_frame/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three exact ordinary/changed reset, call/exception ownership and actual reused handler observation mechanisms.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive frame attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_architecture_frame.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith(('attempt','root_'))}
 for d in reports:
  assert d['status']=='pass' and d['commands'] and all(q['passed'] for q in d['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in d[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in d['source_sha256']}
 d=read(canon());old=read(HERE/(PROJECT+'.before-frame.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;rows=all_rows(r);assert len(rows)==6
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Three original T37.09 criterion/V1 pairs',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_architecture_frame/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact frame dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
