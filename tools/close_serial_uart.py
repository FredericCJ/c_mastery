"""Add source-bound T38.01/.02 support while preserving every original task."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_uart';PROJECT='PR-T38-01';REPORT='research/v5/expansion/legacy_serial_uart/validation.json';SUPPORT='research/v5/expansion/legacy_serial_uart/completion_dispositions.json'
from research.v5.expansion.legacy_serial_uart.author_support import attach
ARTIFACTS=['labs/serial_followthrough/uart/CONTRACTS.md','labs/serial_followthrough/uart/BOARD_CAPTURE.md','assessor/serial_followthrough/uart/ANALYSIS.md','assessor/serial_followthrough/uart/uart_depth.c','assessor/serial_followthrough/uart/cache_investigation.py','labs/serial_followthrough/uart/tests.c','labs/serial_followthrough/uart/mapped.c','assessor/serial_followthrough/uart/oracle.py']
REMAINING=['Physical UART/RS232 waveform and calibrated accuracy with a documented F439 board/peer remain required; changed-M7 hardware port is not claimed.','Other T38 projects remain independently required; no full serial curriculum closure.','External review and independent original/seven-day changed learner attempts remain unperformed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def all_rows(r):
 rows=[];patterns={'T38.01.a':['.mapped','.closure'], 'T38.01.b':['.mapped','cache.','.mutation.AF','.mutation.DMA_CANCEL'],'T38.01.c':['.baud.oracle','.execute','.closure','.mapped'],'T38.02.a':['.frames','.mutation.WIDTH'],'T38.02.b':['.baud','.mutation.GENERATION'],'T38.02.c':['.tx','.rx','.deadline','.mapped','.mutation.EARLY','.mutation.BUDGET','.mutation.ERROR','.mutation.STATUS','.mutation.DMA_TC']}
 why={'T38.01.a':'Source-backed struct lr_cpu/MMIO/controller/transceiver ownership dossier and explicit SPI/MCP2515 changed boundary analysis; no CAN implementation or physical-route substitution.','T38.01.b':'Actual retained F439 pin/DMA aperture code rejects CCM and crossing ranges; changed-M7 whole-line consistency model and explicit missing target-port evidence.','T38.01.c':'Actual native/Thumb/RAM-readback evidence classified separately; authored calibrated-capture protocol and exact +3-percent/local-accuracy counterexample preserve physical obligations.','T38.02.a':'Actual codec composition across2048 supported payload/format states, parity/stop rejection, seven-payload parity and hardware M-field distinctions.','T38.02.b':'Independent Fraction oracle for both BRR packings and three peripheral clocks; actual configured-generation rejection and oversampling lower bound.','T38.02.c':'Actual struct pu_driver/PuRx16-byte/32-slot adapters preserve error/ownership/deadline distinctions; actual mapped DMA flags, EN acknowledgment and final TC plus cancellation.'}
 for c in read(canon())['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:
   dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:2]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=[q['id'] for q in r['commands'] if any(x in q['id'] for x in patterns[c['objective']])]),closure_rationale=why[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 d=read(canon());p=HERE/(PROJECT+'.before-uart.json')
 if not p.exists():p.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/serial_followthrough/uart').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/uart.c','labs/peripheral_depth/uart/target.c','labs/peripheral_depth/uart/CONTRACTS.md','labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/serial_depth/CONTRACTS.md','labs/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T38',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/peripheral_depth/uart/CONTRACTS.md','labs/serial_depth/CONTRACTS.md'],source_register='research/v5/expansion/legacy_serial_uart/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Six exact architecture/UART criterion/V1 pairs; physical observations remain separately required.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive UART attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_serial_uart.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for k in ['source_sha256','outputs_sha256']:
   for n,h in r[k].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 d=read(canon());old=read(HERE/(PROJECT+'.before-uart.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;rows=all_rows(r);assert len(rows)==12
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T38-01 all original ordinary/V1 pairs, six dimensions',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_serial_uart/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact UART dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
