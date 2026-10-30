"""Exact original three SPI criterion/changed pairs, without historical rewrite."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_spi';PROJECTS=['PR-T38-05'];REPORT='research/v5/expansion/legacy_serial_spi/validation.json';SUPPORT='research/v5/expansion/legacy_serial_spi/completion_dispositions.json'
from research.v5.expansion.legacy_serial_spi.author_support import attach
ARTIFACTS=['labs/serial_followthrough/spi/CONTRACTS.md','labs/serial_followthrough/spi/BOARD_CAPTURE.md','assessor/serial_followthrough/spi/ANALYSIS.md','assessor/serial_followthrough/spi/scheduler.c','assessor/serial_followthrough/spi/ports.c','labs/serial_followthrough/spi/tests.c','labs/serial_followthrough/spi/guest.c','labs/serial_followthrough/spi/mapped.c','assessor/serial_followthrough/spi/oracle.py','assessor/serial_followthrough/spi/provider_mutant.py']
REMAINING=['Physical selected-peer edge/timing/electrical and actual F439 SPI/DMA IRQ observations remain required.','Other T38 projects, T39–40 and mixed LRUN parent scope remain separately required.','External review and independent original/seven-day changed learner attempts remain unperformed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(p):return ROOT/('curriculum/v5/expansion/legacy_assessments/'+p+'.json')
def all_rows(r):
 patterns={'T38.06.a':['.edges','.device','.mutation.EDGE','.mutation.ENDIAN','.mutation.STATUS','.mutation.MODE'], 'T38.06.b':['.quiescence','.irq','.mutation.TOKEN','.mutation.CANCEL','.mutation.BSY','.mutation.LEASE','.closure','.stack'], 'T38.06.c':['.queue','.device','.quiescence','.mapped','.mutation.DRAIN','.mutation.LEASE','.mutation.WIDTH','.mutation.CHANNEL']}
 why={'T38.06.a':'All four mode/byte pair traces, actual three-frame peer composition and little-endian payload transfer; wrong sample/status/endian controls.','T38.06.b':'Independent DMA disable acknowledgements, wire/hold ownership and actual cancellation/reset/new-token/late CPU-IRQ sequence using frozen struct ps_spi.','T38.06.c':'Original no-RX-read loss, incomplete dummy-clock framing and fixed copied-slot scheduler composed with actual algo_ring; deadlines never grant storage reuse.'}
 rows=[]
 for p in PROJECTS:
  for c in read(canon(p))['criteria']:
   v=c['variants'][0]
   for delayed in [False,True]:
    dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:2]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    checks=[q['id'] for q in r['commands'] if any(x in q['id'] for x in patterns[c['objective']])]
    if c['objective']=='T38.06.b':checks += [q['id'] for q in r['commands'] if q['id'] in ['arm-none-eabi-gcc.O0.execute','arm-none-eabi-gcc.O3.execute','clang.O0.execute','clang.O3.execute']]
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=p,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=list(dict.fromkeys(checks))),closure_rationale=why[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/serial_followthrough/spi').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/peripheral_depth/spi/spi.h','labs/peripheral_depth/spi/spi.c','labs/peripheral_depth/spi/target.c','labs/peripheral_depth/spi/CONTRACTS.md','labs/peripheral_depth/spi/BOARD_CAPTURE.md','labs/peripheral_depth/uart/uart.h','labs/peripheral_depth/uart/uart.c','labs/peripheral_depth/uart/CONTRACTS.md','labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/algorithms/include/algo.h','labs/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld']
 for p in PROJECTS:
  d=read(canon(p));old=HERE/(p+'.before-spi.json')
  if not old.exists():old.write_bytes(canon(p).read_bytes())
  write(HERE/('attachment_'+p+'.json'),dict(project=p,domain='T38',supplied_artifacts=public,shared_contracts=ARTIFACTS[:2]+['labs/peripheral_depth/spi/CONTRACTS.md','labs/peripheral_depth/spi/BOARD_CAPTURE.md'],source_register='research/v5/expansion/legacy_serial_spi/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three original criterion/V1 pairs; actual author experiments do not discharge physical or independent learner gates.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(p),after)
 print('PASS additive SPI attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_serial_spi.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for k in ['source_sha256','outputs_sha256']:
   for n,h in r[k].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 for p in PROJECTS:
  d=read(canon(p));old=read(HERE/(p+'.before-spi.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(p))
 r,e=reports;rows=all_rows(r);assert len(rows)==6
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T38-05 all three original criterion/V1 pairs, six evidence dimensions',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_serial_spi/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS6 exact SPI dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
