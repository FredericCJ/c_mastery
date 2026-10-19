"""Additive exact original T38.07–.08 criteria and delayed-condition bindings."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_transceivers';PROJECTS=['PR-T38-06','PR-T38-07'];REPORT='research/v5/expansion/legacy_serial_transceivers/validation.json';SUPPORT='research/v5/expansion/legacy_serial_transceivers/completion_dispositions.json'
from research.v5.expansion.legacy_serial_transceivers.author_support import attach
ARTIFACTS=['labs/serial_followthrough/transceivers/CONTRACTS.md','labs/serial_followthrough/transceivers/BOARD_CAPTURE.md','assessor/serial_followthrough/transceivers/ANALYSIS.md','assessor/serial_followthrough/transceivers/transceivers.c','labs/serial_followthrough/transceivers/tests.c','labs/serial_followthrough/transceivers/guest.c','assessor/serial_followthrough/transceivers/oracle.py']
REMAINING=['Physical selected MAX3232 and both differential receiver traces, changed cable/load/baud measurements and instrument uncertainty remain required.','Actual RS485 electrical part/loading/bias/termination/failsafe is not supplied by the trusted coordinator model; PR-T38-08 remains in scope.','Other T38–40 and mixed LRUN parents are not closed by these six criterion pairs.','Independent original/seven-day learner assessment and external review remain unperformed.']
PATTERNS={'T38.07.a':['.thresholds','.mutation.TYPICAL'],'T38.07.b':['.connections','.mutation.DIRECTION'],'T38.07.c':['.capture232','.connections','.mutation.EVIDENCE'],'T38.08.a':['.differential','.mutation.OPEN'],'T38.08.b':['.differential','.exclusion','.mutation.EXCLUSION'],'T38.08.c':['.captures422','.differential','.mutation.CROSSINGS']}
WHY={'T38.07.a':'Actual selected guaranteed-threshold classification versus retained synthetic interval, exhaustive integer boundaries and typical-value counterexample.','T38.07.b':'Original fully documented three-contact DTE/DCE and null-modem adapters, all27 wiring triples and output-output rejection.','T38.07.c':'Actual frozen UaTx/PuDriver/formatter composition, complete synthetic line evidence admission and explicit revised load/rate/capture obligations.','T38.08.a':'Selected signed differential/common-mode envelope and sign adapter, powered-open versus short versus disabled states using current inspected revision.','T38.08.b':'Actual one-driver topology rejection plus distinct trusted three-peer nonwrapping grant/TC/turnaround policy; hardware multipoint remains separately required.','T38.08.c':'Original raw two-line samples distinguish stable complement, repeated crossings and common-mode excursions; no invented physical causal inference.'}
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
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/serial_followthrough/transceivers').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/serial_followthrough/uart/uart_depth.h','labs/serial_followthrough/uart/learner.c','labs/serial_followthrough/uart/CONTRACTS.md','labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/uart.c','labs/peripheral_depth/uart/CONTRACTS.md','labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/serial_depth/CONTRACTS.md','labs/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld']
 for p in PROJECTS:
  d=read(canon(p));old=HERE/(p+'.before-transceivers.json')
  if not old.exists():old.write_bytes(canon(p).read_bytes())
  write(HERE/('attachment_'+p+'.json'),dict(project=p,domain='T38',supplied_artifacts=public,shared_contracts=ARTIFACTS[:2]+['labs/serial_followthrough/uart/CONTRACTS.md'],source_register='research/v5/expansion/legacy_serial_transceivers/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Original six criterion/V1 pairs; selected-source core, real UART composition and explicitly synthetic captures; physical and independent gates remain.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(p),after)
 print('PASS additive transceiver attachments')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_serial_transceivers.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for k in ['source_sha256','outputs_sha256']:
   for n,h in r[k].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 for p in PROJECTS:
  d=read(canon(p));old=read(HERE/(p+'.before-transceivers.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(p))
 r,e=reports;rows=all_rows(r);assert len(rows)==12
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T38-06/07 all six original criterion/V1 pairs, six evidence dimensions',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_serial_transceivers/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact transceiver dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
