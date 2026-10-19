"""Additive exact original T38.09 criteria and delayed-condition bindings."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_rs485';PROJECTS=['PR-T38-08'];REPORT='research/v5/expansion/legacy_serial_rs485/validation.json';SUPPORT='research/v5/expansion/legacy_serial_rs485/completion_dispositions.json'
from research.v5.expansion.legacy_serial_rs485.author_support import attach
ARTIFACTS=['labs/serial_followthrough/rs485/CONTRACTS.md','labs/serial_followthrough/rs485/BOARD_CAPTURE.md','assessor/serial_followthrough/rs485/ANALYSIS.md','assessor/serial_followthrough/rs485/channel.c','labs/serial_followthrough/rs485/tests.c','labs/serial_followthrough/rs485/guest.c','assessor/serial_followthrough/rs485/oracle.py']
REMAINING=['Physical two-node DE/TX/A/B traces, selected board/errata and changed clock/load evidence remain required.','T38 PR09/10, remaining T39/T40 and mixed LRUN parents remain in scope.','The single-foreground-owner channel does not supply a concurrent ISR adapter or malicious-peer exclusion.','Independent original/seven-day learner assessment and external review remain unperformed.']
PATTERNS={'T38.09.a':['.config','.mutation.WAKE'],'T38.09.b':['.timing','.port','.mutation.TC','.mutation.TXE','.mutation.TOKEN','.mutation.POLARITY'],'T38.09.c':['.events','.cancellation','.link','.mutation.LOSS','.mutation.RESERVE','.mutation.PREFLIGHT','.mutation.DMA_STOP']}
WHY={'T38.09.a':'Actual selected-device enabled/shutdown timing and eighth-load model with explicit physical/exclusion limits.','T38.09.b':'Actual UART copied-byte channel, ordinary/wrapping32-bit traces, rejection before mutation, final TC and selected F439 port/image closure.','T38.09.c':'Actual error/backpressure events, retained DMA lease and reserved unavailable/terminal slots; actual EDU7 decoder composition across every loss boundary and stale completion.'}

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(p):return ROOT/('curriculum/v5/expansion/legacy_assessments/'+p+'.json')
def all_rows(r):
 rows=[]
 for p in PROJECTS:
  for c in read(canon(p))['criteria']:
   v=c['variants'][0]
   for delayed in [False,True]:
    dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:2]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    checks=[q['id'] for q in r['commands'] if any(x in q['id'] for x in PATTERNS[c['objective']]) or q['id'] in [cc+'.O'+o+'.execute' for cc in ['arm-none-eabi-gcc','clang'] for o in ['0','3']]]
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=p,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks),closure_rationale=WHY[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/serial_followthrough/rs485').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/serial_followthrough/uart/uart_depth.h','labs/serial_followthrough/uart/learner.c','labs/serial_followthrough/uart/CONTRACTS.md','labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/uart.c','labs/peripheral_depth/uart/CONTRACTS.md','labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/serial_depth/CONTRACTS.md','labs/interrupt_depth/frame/target.S','labs/communications/include/com_edu7.h','labs/communications/learner/edu7.c']
 for p in PROJECTS:
  d=read(canon(p));old=HERE/(p+'.before-rs485.json')
  if not old.exists():old.write_bytes(canon(p).read_bytes())
  write(HERE/('attachment_'+p+'.json'),dict(project=p,domain='T38',supplied_artifacts=public,shared_contracts=ARTIFACTS[:2]+['labs/serial_followthrough/uart/CONTRACTS.md','labs/peripheral_depth/uart/CONTRACTS.md'],source_register='research/v5/expansion/legacy_serial_rs485/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Original three criterion/V1 pairs; actual UART/DMA channel and EDU7 frame composition, selected F439 adapter; physical and independent gates remain.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(p),after)
 print('PASS additive RS485 attachments')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_serial_rs485.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for k in ['source_sha256','outputs_sha256']:
   for n,h in r[k].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 for p in PROJECTS:
  d=read(canon(p));old=read(HERE/(p+'.before-rs485.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(p))
 r,e=reports;rows=all_rows(r);assert len(rows)==6
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T38-08 all three original criterion/V1 pairs, six evidence dimensions',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_serial_rs485/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact RS485 dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
